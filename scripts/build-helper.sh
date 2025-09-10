# Вспомогательные функции сборки для проекта Printcore
# Этот файл содержит общие функции, используемые скриптами сборки
# Использование: source ./scripts/build-helper.sh

# =============================================================================
# ФУНКЦИИ ЦВЕТНОГО ВЫВОДА
# =============================================================================

# Инициализация поддержки цветов
init_colors() {
    # Проверяем, поддерживает ли терминал цвета: -t (terminal) <fd> (stdout)
    if [[ -t 1 ]]; then
        # ANSI коды цветов для красивого вывода
        RED='\033[0;31m'
        GREEN='\033[0;32m'
        YELLOW='\033[1;33m'
        BLUE='\033[0;34m'
        PURPLE='\033[0;35m'
        CYAN='\033[0;36m'
        NC='\033[0m' # Без цвета
    else
        # Без цветов для неинтерактивных терминалов
        RED=''
        GREEN=''
        YELLOW=''
        BLUE=''
        PURPLE=''
        CYAN=''
        NC=''
    fi
}

# Вывод цветного текста
print_color() {
    local color=$1
    shift
    printf "${color}%s${NC}\n" "$*"
}

print_header() {
    print_color "$CYAN" "[BUILD] $*"
}

print_success() {
    print_color "$GREEN" "[SUCCESS] $*"
}

print_error() {
    print_color "$RED" "[ERROR] $*"
}

print_warning() {
    print_color "$YELLOW" "[WARNING] $*"
}

print_info() {
    print_color "$BLUE" "[INFO] $*"
}

print_debug() {
    print_color "$PURPLE" "[DEBUG] $*"
}

# =============================================================================
# ФУНКЦИИ ВАЛИДАЦИИ
# =============================================================================

# Валидация цели сборки
validate_target() {
    local target=$1
    shift
    local valid_targets=("$@")
    
    for valid_target in "${valid_targets[@]}"; do
        if [[ "$target" == "$valid_target" ]]; then
            return 0
        fi
    done
    return 1
}

# Валидация типов санитайзеров
validate_sanitizers() {
    local sanitizers=$1
    shift
    local valid_sanitizers=("$@")
    
    IFS=',' read -ra SANITIZER_ARRAY <<< "$sanitizers"
    
    # Проверяем каждый санитайзер на валидность
    for sanitizer in "${SANITIZER_ARRAY[@]}"; do
        # Убираем пробелы
        sanitizer=$(echo "$sanitizer" | xargs)
        local valid=false
        for valid_sanitizer in "${valid_sanitizers[@]}"; do
            if [[ "$sanitizer" == "$valid_sanitizer" ]]; then
                valid=true
                break
            fi
        done
        if [[ "$valid" == "false" ]]; then
            print_error "Неверный санитайзер: $sanitizer"
            print_info "Валидные санитайзеры: ${valid_sanitizers[*]}"
            return 1
        fi
    done
    
    # Проверяем конфликты
    local has_thread=false
    local has_address=false
    local has_leak=false
    
    for sanitizer in "${SANITIZER_ARRAY[@]}"; do
        sanitizer=$(echo "$sanitizer" | xargs)
        case "$sanitizer" in
            "thread") has_thread=true ;;
            "address") has_address=true ;;
            "leak") has_leak=true ;;
        esac
    done
    
    if [[ "$has_thread" == "true" ]] && ([[ "$has_address" == "true" ]] || [[ "$has_leak" == "true" ]]); then
        print_error "ThreadSanitizer конфликтует с AddressSanitizer и LeakSanitizer"
        print_info "Используйте 'thread' отдельно или комбинируйте 'address,leak,undefined'"
        return 1
    fi
    
    return 0
}

# Валидация типа сборки
validate_build_type() {
    local build_type=$1
    shift
    local valid_types=("$@")
    
    for valid_type in "${valid_types[@]}"; do
        if [[ "$build_type" == "$valid_type" ]]; then
            return 0
        fi
    done
    return 1
}


# =============================================================================
# ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ
# =============================================================================

# Получить директорию сборки на основе типа сборки
get_build_dir() {
    local build_type=$1
    local build_type_lower=$(echo "$build_type" | tr '[:upper:]' '[:lower:]')
    echo "$(pwd)/build-$build_type_lower"
}

# Проверить, находимся ли мы в корне проекта
check_project_root() {
    if [[ ! -f "CMakeLists.txt" ]]; then
        print_error "Пожалуйста, запустите этот скрипт из корневой директории проекта"
        exit 1
    fi
}

# Получить количество ядер CPU для параллельной сборки
get_cpu_cores() {
    local cores=$(nproc 2>/dev/null || echo "1")
    echo "$cores"
}

# Проверить, существует ли команда
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Проверить, существует ли файл
file_exists() {
    [[ -f "$1" ]]
}

# Проверить, существует ли директория
dir_exists() {
    [[ -d "$1" ]]
}

# =============================================================================
# УПРАВЛЕНИЕ ЗАВИСИМОСТЯМИ
# =============================================================================

# Установка зависимостей
install_dependencies() {
    local deps_array=("$@")
    
    # Если не переданы зависимости, используем пустой массив
    if [[ ${#deps_array[@]} -eq 0 ]]; then
        print_warning "Не переданы зависимости для установки"
        return 1
    fi
    
    print_header "Установка зависимостей для сборки..."
    
    # Проверяем, запущен ли скрипт от root
    if [[ $EUID -ne 0 ]]; then
        print_error "Установка требует привилегий root. Пожалуйста, запустите с sudo."
        return 1
    fi

    print_info "Обновление списков пакетов..."
    apt-get update || {
        print_error "Не удалось обновить списки пакетов"
        return 1
    }

    print_info "Установка пакетов: ${deps_array[*]}"
    apt-get install -y "${deps_array[@]}" || {
        print_error "Не удалось установить необходимые пакеты"
        return 1
    }

    print_success "Зависимости установлены успешно"
    return 0
}

# =============================================================================
# КОНФИГУРАЦИЯ CMAKE
# =============================================================================

# Простая функция для CMake: только build_dir и массив аргументов
configure_cmake() {
    local build_dir=$1
    shift
    local cmake_args=("$@")
    print_info "Конфигурация CMake: ${cmake_args[*]}"
    cmake "${cmake_args[@]}" -B "$build_dir"
}

# Расширенная функция (для build-dev.sh)
configure_cmake_extended() {
    local target=$1
    local build_dir=$2
    local build_type=${3:-"Release"}
    local astra_mode=${4:-"false"}
    local enable_sanitizers=${5:-"false"}
    local sanitizer_types=${6:-"address"}
    
    local cmake_args=(
        "-DCMAKE_BUILD_TYPE=$build_type"
    )
    
    if [[ "$astra_mode" == "true" ]]; then
        cmake_args+=("-DBUILD_FOR_ASTRA=ON")
    fi

    if [[ "$astra_mode" == "true" ]]; then
        cmake_args+=("-DCMAKE_INSTALL_PREFIX=/opt/di/diprint/")
    else
        cmake_args+=("-DCMAKE_INSTALL_PREFIX=/opt/printer")
    fi
    
    # Добавляем конфигурацию санитайзеров
    if [[ "$enable_sanitizers" == "true" ]]; then
        cmake_args+=("-DENABLE_SANITIZERS=ON")
        local cmake_sanitizers=$(echo "$sanitizer_types" | tr ',' ';')
        cmake_args+=("-DSANITIZER_TYPES=$cmake_sanitizers")
        if [[ "$build_type" != "Release" ]]; then
            print_warning "Использование санитайзеров с $build_type сборкой. Рекомендуется использовать --release для оптимальных результатов."
        fi
        print_info "Санитайзеры включены: $sanitizer_types (только исполняемый файл)"
    fi
    
    case "$target" in
        "static-lib")
            cmake_args+=("-DBUILD_STATIC_LIB=ON" "-DBUILD_DYNAMIC_LIB=OFF" "-DBUILD_UTILITY=OFF" "-DBUILD_TESTS=OFF")
            ;;
        "dynamic-lib")
            cmake_args+=("-DBUILD_STATIC_LIB=OFF" "-DBUILD_DYNAMIC_LIB=ON" "-DBUILD_UTILITY=OFF" "-DBUILD_TESTS=OFF")
            ;;
        "executable")
            cmake_args+=("-DBUILD_STATIC_LIB=OFF" "-DBUILD_DYNAMIC_LIB=OFF" "-DBUILD_UTILITY=ON" "-DBUILD_TESTS=OFF")
            ;;
        "tests")
            cmake_args+=("-DBUILD_STATIC_LIB=ON" "-DBUILD_DYNAMIC_LIB=OFF" "-DBUILD_UTILITY=OFF" "-DBUILD_TESTS=ON")
            ;;
        "package")
            cmake_args+=("-DBUILD_STATIC_LIB=ON" "-DBUILD_DYNAMIC_LIB=ON" "-DBUILD_UTILITY=ON" "-DBUILD_TESTS=OFF" "-DBUILD_DEB_PACKAGE=ON")
            ;;
        "all")
            cmake_args+=("-DBUILD_STATIC_LIB=ON" "-DBUILD_DYNAMIC_LIB=ON" "-DBUILD_UTILITY=ON" "-DBUILD_TESTS=ON" "-DBUILD_DEB_PACKAGE=ON")
            ;;
    esac

    # If Astra mode, force -DBUILD_TESTS=OFF and remove any -DBUILD_TESTS=ON
    if [[ "$astra_mode" == "true" ]]; then
        # Remove any -DBUILD_TESTS=ON
        for i in "${!cmake_args[@]}"; do
            if [[ "${cmake_args[$i]}" == "-DBUILD_TESTS=ON" ]]; then
                unset 'cmake_args[$i]'
            fi
        done
        # Add -DBUILD_TESTS=OFF if not already present
        local found_tests_off=false
        for arg in "${cmake_args[@]}"; do
            if [[ "$arg" == "-DBUILD_TESTS=OFF" ]]; then
                found_tests_off=true
                break
            fi
        done
        if [[ "$found_tests_off" == false ]]; then
            cmake_args+=("-DBUILD_TESTS=OFF")
        fi
    fi
    
    print_info "Конфигурация CMake: ${cmake_args[*]}"
    cmake "${cmake_args[@]}" -B "$build_dir"
}

# =============================================================================
# ФУНКЦИИ СБОРКИ
# =============================================================================

# Сборка цели с помощью CMake
build_target_cmake() {
    local target=$1
    local build_dir=$2
    local jobs=$3
    local verbose=$4
    
    print_header "Сборка цели: $target"
    
    # Сборка
    local build_args=("--build" "$build_dir" "-j$jobs")
    if [[ "$verbose" == "true" ]]; then
        build_args+=("--verbose")
    fi
    
    print_info "Сборка с $jobs параллельными задачами..."
    cmake "${build_args[@]}"
}

# Очистка директории сборки
clean_build_dir() {
    local build_dir=$1
    local clean_build=$2
    
    if [[ "$clean_build" == "true" ]]; then
        print_info "Очистка директории сборки: $build_dir"
        rm -rf "$build_dir"
    fi
}

# Запуск тестов
run_tests() {
    local build_dir=$1

    print_info "Запуск тестов..."
    cd "$build_dir"
    ctest --test-dir tests --output-on-failure
    local status=$?
    cd - > /dev/null
    return $status
}

# Создание пакета
create_package() {
    local build_dir=$1

    print_info "Создание DEB пакета..."
    # Сравниваем версии CPack для вызова с корректными командами
    local cpack_version_raw
    cpack_version_raw=$(cpack --version 2>&1 | grep -Eo '[0-9]+\.[0-9]+\.[0-9]+' | head -n1)
    # print_debug "Raw CPack version output: $cpack_version_raw"
    local cpack_version
    cpack_version=$(echo "$cpack_version_raw" | grep -Eo '[0-9]+\.[0-9]+\.[0-9]+' | head -n1)
    # print_debug "Parsed CPack `version: $cpack_version"

    print_info "CPack version: $cpack_version"
    if version_ge "$cpack_version" "3.21.0"; then
        cpack -G DEB --config "$build_dir/CPackConfig.cmake" -D CPACK_PACKAGE_DIRECTORY="$build_dir"
    else
        cd "$build_dir"
        cpack -G DEB
    fi

    local packages=$(find "$build_dir" -name "*.deb" 2>/dev/null || true)
    if [[ -n "$packages" ]]; then
        print_success "Созданные пакеты:"
        echo "$packages"
    fi
}

# =============================================================================
# СПРАВКА ПО САНИТАЙЗЕРАМ
# =============================================================================

# Вывод советов по использованию санитайзеров
print_sanitizer_tips() {
    local sanitizer_types=$1
    
    print_info ""
    print_info "=== СОВЕТЫ ПО ИСПОЛЬЗОВАНИЮ САНИТАЙЗЕРОВ ==="
    
    IFS=',' read -ra SANITIZER_ARRAY <<< "$sanitizer_types"
    for sanitizer in "${SANITIZER_ARRAY[@]}"; do
        sanitizer=$(echo "$sanitizer" | xargs)
        case "$sanitizer" in
            "address")
                print_info "AddressSanitizer включен. Он обнаружит:"
                print_info "  • Переполнения и недополнения буферов"
                print_info "  • Ошибки использования после освобождения"
                print_info "  • Утечки памяти (с detect_leaks=1)"
                print_info "  • Ошибки двойного освобождения"
                ;;
            "thread")
                print_info "ThreadSanitizer включен. Он обнаружит:"
                print_info "  • Гонки данных между потоками"
                print_info "  • Взаимные блокировки"
                print_info "  • Нарушения потокобезопасности"
                ;;
            "memory")
                print_info "MemorySanitizer включен. Он обнаружит:"
                print_info "  • Использование неинициализированной памяти"
                print_info "  • Требует компилятор Clang"
                ;;
            "undefined")
                print_info "UndefinedBehaviorSanitizer включен. Он обнаружит:"
                print_info "  • Переполнение целых чисел"
                print_info "  • Нарушения границ массивов"
                print_info "  • Разыменование нулевых указателей"
                ;;
            "leak")
                print_info "LeakSanitizer включен. Он обнаружит:"
                print_info "  • Утечки памяти при выходе из программы"
                ;;
        esac
    done
    
    print_info "Запускайте программу, как обычно, - санитайзеры будут сообщать о проблемах в stderr"
    print_info "==============================================="
}

# =============================================================================
# ВСПОМОГАТЕЛЬНАЯ ФУНКЦИЯ СРАВНЕНИЯ ВЕРСИЙ
# =============================================================================
# Возвращает 0 если $1 >= $2
version_ge() {
    [ "$(printf '%s\n' "$2" "$1" | sort -V | head -n1)" = "$2" ]
}

# =============================================================================
# INITIALIZATION
# =============================================================================

# Initialize the helper (call this when sourcing)
init_build_helper() {
    init_colors
    # print_debug "Build helper functions loaded"
}

# Auto-initialize when sourced
init_build_helper 


#############################################
# Decide if package creation is needed
# Usage: is_package_creation_needed <cmake_args_array_name> <build_target_string>
# Returns 0 (true) if packaging should be performed, 1 (false) otherwise
is_package_creation_needed() {
    # First arg is the NAME of an array variable that holds CMake arguments
    # We use nameref to access it without copying
    local -n __cmake_args_ref=$1
    local __build_target="$2"

    local __need=false

    # 1) Explicit CMake switch controls packaging
    for __arg in "${__cmake_args_ref[@]}"; do
        case "$__arg" in
            -DBUILD_DEB_PACKAGE=ON|-DBUILD_DEB_PACKAGE=TRUE|-DBUILD_DEB_PACKAGE=1)
                __need=true
                break
                ;;
        esac
    done

    # 2) Targets that imply packaging (convention in our scripts)
    if [[ "$__need" == false ]]; then
        if [[ "$__build_target" == *"package"* ]] || [[ "$__build_target" == *"all"* ]]; then
            __need=true
        fi
    fi

    # Return like a predicate: 0 means "yes, do it"
    if [[ "$__need" == true ]]; then
        return 0
    else
        return 1
    fi
}
