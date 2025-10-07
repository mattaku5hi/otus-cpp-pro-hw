
#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "aggregator.h"
#include "ibulk_listener.h"
#include "notifier.h"


using namespace bulkapp;


namespace 
{

struct TestSink : IBulkListener 
{
    std::vector<Bulk> got;
    void onBulk(const Bulk& b) override 
    { 
        got.push_back(b); 
    }
};

} // namespace


TEST(AggregatorTest, StaticBlockExactN) 
{
    Notifier notifier;
    auto sink = std::make_shared<TestSink>();
    notifier.subscribe(sink);

    Aggregator agg{3, notifier};
    agg.onLine("cmd1");
    agg.onLine("cmd2");
    agg.onLine("cmd3");

    ASSERT_EQ(sink->got.size(), 1u);
    EXPECT_EQ(sink->got[0].commands, (std::vector<std::string>{"cmd1", "cmd2", "cmd3"}));
}

TEST(AggregatorTest, StaticBlockFlushAtEof) 
{
    Notifier notifier;
    auto sink = std::make_shared<TestSink>();
    notifier.subscribe(sink);

    Aggregator agg{3, notifier};
    agg.onLine("cmd4");
    agg.onLine("cmd5");
    agg.onEof();

    ASSERT_EQ(sink->got.size(), 1u);
    EXPECT_EQ(sink->got[0].commands, (std::vector<std::string>{"cmd4", "cmd5"}));
}

TEST(AggregatorTest, DynamicBlocksIgnoreNestedBraces) 
{
    Notifier notifier;
    auto sink = std::make_shared<TestSink>();
    notifier.subscribe(sink);

    Aggregator agg{3, notifier};
    agg.onLine("cmd1");
    agg.onLine("cmd2");
    agg.onLine("{"); // start dynamic -> flush previous static (cmd1, cmd2)
    ASSERT_EQ(sink->got.size(), 1u);
    EXPECT_EQ(sink->got[0].commands, (std::vector<std::string>{"cmd1", "cmd2"}));

    agg.onLine("cmd3");
    agg.onLine("cmd4");
    agg.onLine("}"); // end dynamic -> flush (cmd3, cmd4)

    ASSERT_EQ(sink->got.size(), 2u);
    EXPECT_EQ(sink->got[1].commands, (std::vector<std::string>{"cmd3", "cmd4"}));
}

TEST(AggregatorTest, DynamicUnfinishedIsIgnored) 
{
    Notifier notifier;
    auto sink = std::make_shared<TestSink>();
    notifier.subscribe(sink);

    Aggregator agg{3, notifier};
    agg.onLine("{");
    agg.onLine("cmd10");
    agg.onLine("cmd11");
    agg.onEof(); // unfinished dynamic -> ignore

    ASSERT_TRUE(sink->got.empty());
}

