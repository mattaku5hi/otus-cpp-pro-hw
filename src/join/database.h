#pragma once

#include <condition_variable>
#include <cstddef>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

#include <sqlite3.h>

namespace async 
{

namespace join 
{


constexpr const char* pDbPath = "file:join?mode=memory&cache=shared";

struct SqliteStmtDeleter
{
    void operator()(sqlite3_stmt* p) const noexcept
    {
        if(p != nullptr)
        {
            sqlite3_finalize(p);
        }
    }
};

struct SqliteDbDeleter
{
    void operator()(sqlite3* p) const noexcept
    {
        if(p != nullptr)
        {
            sqlite3_close(p);
        }
    }
};

using SqliteStmtPtr = std::unique_ptr<sqlite3_stmt, SqliteStmtDeleter>;
using SqliteDbPtr = std::unique_ptr<sqlite3, SqliteDbDeleter>;

class Database 
{
public:
    explicit Database(std::string filePath = std::string(pDbPath), std::size_t poolSize = 4)
        : m_filePath(std::move(filePath))
    {
        if(m_filePath == ":memory:")
        {
            m_filePath = pDbPath;
        }
        std::string err;
        if(this->initPool(poolSize, err) == false)
        {
            m_pool.clear();
            m_ready = false;
            m_initErr = std::move(err);
        }
        else
        {
            m_ready = true;
        }
    }

    ~Database()
    {
        // Connections are closed automatically by SqliteDbDeleter.
    }

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    // tableId: 'A' or 'B'
    bool insert(char tableId, int id, const std::string& name, std::string& err)
    {
        // Convert 'A'/'B' to a real table name. We do this before touching SQLite.
        const auto tbl = this->tableName(tableId, err);
        if(tbl.has_value() == false)
        {
            return false;
        }

        auto lease = this->acquire(err);
        if(!lease)
        {
            return false;
        }
        sqlite3* db = lease.get();

        sqlite3_stmt* rawStmt = nullptr;
        const std::string sql = "INSERT INTO " + *tbl + "(id, name) VALUES(?, ?);";
        // Prepare the SQL into a bytecode statement ("compiled" query). This is the standard SQLite pattern:
        // prepare -> bind parameters -> step -> finalize.
        if(sqlite3_prepare_v2(db, sql.c_str(), -1, &rawStmt, nullptr) != SQLITE_OK)
        {
            err = sqlite3_errmsg(db);
            return false;
        }

        // Every sqlite3_prepare_v2() must be paired with sqlite3_finalize() to avoid leaks.
        // RAII
        SqliteStmtPtr stmt(rawStmt);

        // Bind parameters by index (1-based): first ? is id, second ? is name.
        if(sqlite3_bind_int(stmt.get(), 1, id) != SQLITE_OK)
        {
            err = sqlite3_errmsg(db);
            return false;
        }
        // SQLITE_TRANSIENT tells SQLite to copy the string, so it's safe after this call returns.
        if(sqlite3_bind_text(stmt.get(), 2, name.c_str(), static_cast<int>(name.size()), SQLITE_TRANSIENT) != SQLITE_OK)
        {
            err = sqlite3_errmsg(db);
            return false;
        }

        // Execute. For INSERT/UPDATE/DELETE, expected return is SQLITE_DONE.
        const int rc = sqlite3_step(stmt.get());
        if(rc != SQLITE_DONE)
        {
            // Our protocol expects a custom error message for duplicates
            // Check for constraint failures (duplicates)
            if(rc == SQLITE_CONSTRAINT || rc == SQLITE_CONSTRAINT_PRIMARYKEY || rc == SQLITE_CONSTRAINT_UNIQUE)
            {
                err = std::string("duplicate ") + std::to_string(id);
            }
            else
            {
                err = sqlite3_errmsg(db);
            }
            return false;
        }

        return true;
    }

    bool truncate(char tableId, std::string& err)
    {
        const auto tbl = this->tableName(tableId, err);
        if(tbl.has_value() == false)
        {
            return false;
        }

        // Serialize access to the single connection.
        auto lease = this->acquire(err);
        if(!lease)
        {
            return false;
        }
        sqlite3* db = lease.get();

        const std::string sql = "DELETE FROM " + *tbl + ";";
        // sqlite3_exec() is a convenience API for statements without parameters.
        return this->exec(db, sql.c_str(), err);
    }

    // Returns sorted by id
    std::vector<std::tuple<int, std::string, std::string>> intersection() const
    {
        std::vector<std::tuple<int, std::string, std::string>> out;
        std::string err;

        // Even though this method is const, we still need to lock for thread-safety.
        auto lease = this->acquire(err);
        if(!lease)
        {
            return out;
        }
        sqlite3* db = lease.get();

        static constexpr const char* sql =
            "SELECT A.id, A.name, B.name "
            "FROM A JOIN B ON A.id = B.id "
            "ORDER BY A.id;";

        sqlite3_stmt* rawStmt = nullptr;
        if(sqlite3_prepare_v2(db, sql, -1, &rawStmt, nullptr) != SQLITE_OK)
        {
            return out;
        }

        SqliteStmtPtr stmt(rawStmt);

        // For SELECT queries, sqlite3_step() returns:
        // - SQLITE_ROW while there are rows available
        // - SQLITE_DONE when the result set is finished
        while(true)
        {
            const int rc = sqlite3_step(stmt.get());
            if(rc == SQLITE_ROW)
            {
                const int id = sqlite3_column_int(stmt.get(), 0);
                // sqlite3_column_text() returns a pointer into SQLite-managed memory.
                // It remains valid until the next sqlite3_step() / sqlite3_finalize() on this statement.
                const unsigned char* aName = sqlite3_column_text(stmt.get(), 1);
                const unsigned char* bName = sqlite3_column_text(stmt.get(), 2);
                out.emplace_back(
                    id,
                    aName ? reinterpret_cast<const char*>(aName) : std::string(),
                    bName ? reinterpret_cast<const char*>(bName) : std::string());
                continue;
            }
            break;
        }

        return out;
    }

    // Returns sorted by id
    std::vector<std::tuple<int, std::string, std::string>> diffSymmetric() const
    {
        std::vector<std::tuple<int, std::string, std::string>> out;
        std::string err;

        // Serialize read queries too, since the same sqlite3* handle is shared.
        auto lease = this->acquire(err);
        if(!lease)
        {
            return out;
        }
        sqlite3* db = lease.get();

        // Symmetric difference = rows that exist only in A or only in B.
        // We implement it as:
        // 1) A \ B (LEFT JOIN + WHERE B.id IS NULL)
        // 2) B \ A
        // and then UNION ALL results.
        // Empty string '' is used as a placeholder for the missing name.
        static constexpr const char* sql =
            "SELECT A.id, A.name, '' "
            "FROM A LEFT JOIN B ON A.id = B.id "
            "WHERE B.id IS NULL "
            "UNION ALL "
            "SELECT B.id, '', B.name "
            "FROM B LEFT JOIN A ON A.id = B.id "
            "WHERE A.id IS NULL "
            "ORDER BY 1;";

        sqlite3_stmt* rawStmt = nullptr;
        if(sqlite3_prepare_v2(db, sql, -1, &rawStmt, nullptr) != SQLITE_OK)
        {
            return out;
        }

        SqliteStmtPtr stmt(rawStmt);

        while(true)
        {
            const int rc = sqlite3_step(stmt.get());
            if(rc == SQLITE_ROW)
            {
                // Column order matches the SELECT list above.
                const int id = sqlite3_column_int(stmt.get(), 0);
                const unsigned char* aName = sqlite3_column_text(stmt.get(), 1);
                const unsigned char* bName = sqlite3_column_text(stmt.get(), 2);
                out.emplace_back(
                    id,
                    aName ? reinterpret_cast<const char*>(aName) : std::string(),
                    bName ? reinterpret_cast<const char*>(bName) : std::string());
                continue;
            }
            break;
        }
        return out;
    }

private:
    class DbLease
    {
    public:
        DbLease() = default;

        DbLease(const DbLease&) = delete;
        DbLease& operator=(const DbLease&) = delete;

        DbLease(DbLease&& other) noexcept
            : m_owner(other.m_owner), m_db(std::move(other.m_db))
        {
            other.m_owner = nullptr;
        }

        DbLease& operator=(DbLease&& other) noexcept
        {
            if(this != &other)
            {
                this->release();
                m_owner = other.m_owner;
                m_db = std::move(other.m_db);
                other.m_owner = nullptr;
            }
            return *this;
        }

        ~DbLease()
        {
            this->release();
        }

        explicit operator bool() const noexcept
        {
            return static_cast<bool>(m_db);
        }

        sqlite3* get() const noexcept
        {
            return m_db.get();
        }

    private:
        friend class Database;

        DbLease(const Database* owner, SqliteDbPtr db)
            : m_owner(owner), m_db(std::move(db))
        {
        }

        void release()
        {
            if(m_owner == nullptr || m_db == nullptr)
            {
                return;
            }
            m_owner->release(std::move(m_db));
            m_owner = nullptr;
        }

        const Database* m_owner{nullptr};
        SqliteDbPtr m_db;
    };

    bool initPool(std::size_t poolSize, std::string& err)
    {
        if(poolSize == 0)
        {
            poolSize = 1;
        }

        std::lock_guard lk(m_poolMtx);
        m_pool.reserve(poolSize);
        for(std::size_t i = 0; i < poolSize; ++i)
        {
            SqliteDbPtr db;
            if(this->openConnection(db, err) == false)
            {
                return false;
            }
            m_pool.emplace_back(std::move(db));
        }
        return true;
    }

    DbLease acquire(std::string& err) const
    {
        std::unique_lock lk(m_poolMtx);
        if(m_ready == false)
        {
            err = m_initErr.empty() ? std::string("sqlite pool is not initialized") : m_initErr;
            return DbLease{};
        }
        m_poolCv.wait(lk, [this]() { 
            return m_pool.empty() == false; 
        });
        SqliteDbPtr db = std::move(m_pool.back());
        m_pool.pop_back();
        if(!db)
        {
            err = "sqlite connection pool returned null";
            return DbLease{};
        }
        return DbLease(this, std::move(db));
    }

    void release(SqliteDbPtr db) const
    {
        {
            std::lock_guard lk(m_poolMtx);
            m_pool.emplace_back(std::move(db));
        }
        m_poolCv.notify_one();
    }

    bool openConnection(SqliteDbPtr& outDb, std::string& err) const
    {
        sqlite3* raw = nullptr;
        const int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_URI;
        if(sqlite3_open_v2(m_filePath.c_str(), &raw, flags, nullptr) != SQLITE_OK)
        {
            err = raw ? sqlite3_errmsg(raw) : std::string("sqlite3_open_v2 failed");
            if(raw)
            {
                sqlite3_close(raw);
            }
            return false;
        }

        // Avoid long SQLITE_BUSY failures under contention.
        sqlite3_busy_timeout(raw, 1000);

        outDb.reset(raw);

        const bool inMemory = (m_filePath.find("mode=memory") != std::string::npos);
        if(inMemory)
        {
            if(exec(raw, "PRAGMA journal_mode=MEMORY;", err) == false)
            {
                outDb.reset();
                return false;
            }
        }
        else if(exec(raw, "PRAGMA journal_mode=WAL;", err) == false)
        {
            outDb.reset();
            return false;
        }
        if(exec(raw, "PRAGMA synchronous=NORMAL;", err) == false)
        {
            outDb.reset();
            return false;
        }
        if(exec(raw, "CREATE TABLE IF NOT EXISTS A(id INTEGER PRIMARY KEY, name TEXT NOT NULL);", err) == false)
        {
            outDb.reset();
            return false;
        }
        if(exec(raw, "CREATE TABLE IF NOT EXISTS B(id INTEGER PRIMARY KEY, name TEXT NOT NULL);", err) == false)
        {
            outDb.reset();
            return false;
        }

        return true;
    }

    static std::optional<std::string> tableName(char tableId, std::string& err)
    {
        // We accept only two tables in our protocol.
        if(tableId == 'A')
        {
            return std::string("A");
        }
        if(tableId == 'B')
        {
            return std::string("B");
        }

        err = "unknown table";
        return std::nullopt;
    }

    bool exec(sqlite3* db, const char* sql, std::string& err) const
    {
        // Convenience API: executes SQL directly (no parameters / no result processing).
        // We only use it for PRAGMAs and simple DDL/DML.
        char* sqliteErr = nullptr;
        const int rc = sqlite3_exec(db, sql, nullptr, nullptr, &sqliteErr);
        if(rc != SQLITE_OK)
        {
            if(sqliteErr)
            {
                err = sqliteErr;
                sqlite3_free(sqliteErr);
            }
            else
            {
                err = sqlite3_errmsg(db);
            }
            return false;
        }
        return true;
    }

    std::string m_filePath;
    mutable std::mutex m_poolMtx;
    mutable std::condition_variable m_poolCv;
    mutable std::vector<SqliteDbPtr> m_pool;
    bool m_ready{false};
    std::string m_initErr;
};

} // namespace join

} // namespace async
