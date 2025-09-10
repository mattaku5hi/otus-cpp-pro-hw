#!/bin/bash
set -e
set -u

# Унифицированный скрипт сборки для проекта Printcore
# Поддерживает множественные цели сборки и конфигурации
# Использование: ./scripts/build-dev.sh [OPTIONS] [TARGET]

# Подключаем вспомогательные функции
source "$(dirname "$0")/build-helper.sh"

# Инициализируем цвета
init_colors

# Конфигурация по умолчанию
BUILD_TYPE="Release"
BUILD_DIR=""
INSTALL_DEPS=false
CLEAN_BUILD=true
VERBOSE=false
JOBS=$(get_cpu_cores)
DEVICE_PATH="/dev/ttyACM0"
ASTRA_MODE=false
ENABLE_SANITIZERS=false
SANITIZER_TYPES="address"

# Доступные цели
VALID_TARGETS=("static-lib" "dynamic-lib" "executable" "tests" "all" "package")

# Доступные типы санитайзеров
VALID_SANITIZERS=("address" "thread" "memory" "undefined" "leak")

# Доступные типы сборки
VALID_BUILD_TYPES=("Debug" "Release" "RelWithDebInfo" "MinSizeRel")

# Список зависимостей для установки
BUILD_DEPS=(
    build-essential
    cmake
    # libnetpbm10-dev
    libgtest-dev
    libgmock-dev
    libfmt-dev
    libspdlog-dev
    libsystemd-dev
    pkg-config
)

# Информация об использовании
usage() {
    printf "${CYAN}Унифицированный скрипт сборки для проекта Printcore${NC}\n\n"
    
    printf "${YELLOW}ИСПОЛЬЗОВАНИЕ:${NC}\n"
    printf "    %s [OPTIONS] [TARGET]\n\n" "$(basename "$0")"
    
    printf "${YELLOW}ЦЕЛИ:${NC}\n"
    printf "    ${GREEN}static-lib${NC}     Сборка только статической библиотеки\n"
    printf "    ${GREEN}dynamic-lib${NC}    Сборка только динамической библиотеки\n"
    printf "    ${GREEN}executable${NC}     Сборка только исполняемого файла/утилиты\n"
    printf "    ${GREEN}tests${NC}          Сборка и запуск набора тестов\n"
    printf "    ${GREEN}all${NC}            Сборка статической библиотеки + исполняемого файла + тестов\n"
    printf "    ${GREEN}package${NC}        Сборка исполняемого файла и создание DEB пакета\n\n"
    
    printf "${YELLOW}ОПЦИИ:${NC}\n"
    printf "    ${GREEN}--install${NC}              Установка необходимых зависимостей (требует sudo)\n"
    printf "    ${GREEN}--build-type TYPE${NC}      Тип сборки: Debug | Release | RelWithDebInfo | MinSizeRel (по умолчанию: Release)\n"
    printf "    ${GREEN}--release${NC}              Тип сборки: Release (по умолчанию)\n"
    printf "    ${GREEN}--debug${NC}                Тип сборки: Debug\n"
    printf "    ${GREEN}--relwithdebinfo${NC}       Тип сборки: RelWithDebInfo\n"
    printf "    ${GREEN}--minsize${NC}              Тип сборки: MinSizeRel\n"
    printf "    ${GREEN}--device PATH${NC}          Установка пути USB устройства (по умолчанию: /dev/ttyACM0)\n"
    printf "    ${GREEN}--astra${NC}                Сборка для Astra Linux (исключает libfmt)\n"
    printf "    ${GREEN}--sanitizer TYPES${NC}       Включение санитайзеров: address,thread,memory,undefined,leak (через запятую)\n"
    printf "    ${GREEN}--no-sanitizer${NC}         Отключение санитайзеров (по умолчанию)\n"
    printf "    ${GREEN}--no-clean${NC}             Не очищать директорию сборки перед сборкой\n"
    printf "    ${GREEN}--verbose${NC}              Включение подробного вывода сборки\n"
    printf "    ${GREEN}--jobs N${NC}               Количество параллельных задач сборки (по умолчанию: %d)\n" "$(get_cpu_cores)"
    printf "    ${GREEN}-h, --help${NC}             Показать это сообщение помощи\n\n"
    
    printf "${YELLOW}ТИПЫ САНИТАЙЗЕРОВ:${NC}\n"
    printf "    ${GREEN}address${NC}        AddressSanitizer - обнаруживает ошибки памяти (по умолчанию)\n"
    printf "    ${GREEN}thread${NC}         ThreadSanitizer - обнаруживает гонки данных и взаимные блокировки\n"
    printf "    ${GREEN}memory${NC}         MemorySanitizer - обнаруживает чтение неинициализированной памяти (только Clang)\n"
    printf "    ${GREEN}undefined${NC}      UndefinedBehaviorSanitizer - обнаруживает неопределенное поведение\n"
    printf "    ${GREEN}leak${NC}           LeakSanitizer - обнаруживает утечки памяти\n\n"
    
    printf "${YELLOW}КОМБИНАЦИИ САНИТАЙЗЕРОВ:${NC}\n"
    printf "    ${GREEN}address,leak${NC}          Комплексное обнаружение ошибок памяти\n"
    printf "    ${GREEN}address,undefined${NC}     Ошибки памяти + неопределенное поведение\n"
    printf "    ${GREEN}address,leak,undefined${NC} Полная проверка памяти и поведения\n"
    printf "    ${GREEN}thread${NC}                Потокобезопасность (конфликтует с address/leak)\n\n"
    
    printf "${YELLOW}ПРИМЕРЫ:${NC}\n"
    printf "    %s static-lib                    # Сборка статической библиотеки\n" "$(basename "$0")"
    printf "    %s --build-type Debug executable # Сборка исполняемого файла в режиме Debug\n" "$(basename "$0")"
    printf "    %s --build-type RelWithDebInfo executable # Сборка с RelWithDebInfo (-O2 -g -fno-omit-frame-pointer)\n" "$(basename "$0")"
    printf "    %s --build-type Release --sanitizer address executable # Сборка исполняемого файла с AddressSanitizer\n" "$(basename "$0")"
    printf "    %s --build-type RelWithDebInfo executable      # Сборка исполняемого файла с RelWithDebInfo (-O2 -g -fno-omit-frame-pointer)\n" "$(basename "$0")"
    printf "    %s --build-type Release --sanitizer address,leak executable # Сборка с Address+LeakSanitizer\n" "$(basename "$0")"
    printf "    %s --build-type Release --sanitizer thread executable # Сборка исполняемого файла с ThreadSanitizer\n" "$(basename "$0")"
    printf "    %s --install                     # Только установка зависимостей\n" "$(basename "$0")"
    printf "    %s --device /dev/ttyUSB0 all     # Сборка всего с пользовательским устройством\n" "$(basename "$0")"
    printf "    %s tests                         # Сборка и запуск тестов\n\n" "$(basename "$0")"
    
    printf "${YELLOW}ДИРЕКТОРИИ СБОРКИ:${NC}\n"
    printf "    build-release/     Release сборки\n"
    printf "    build-debug/       Debug сборки\n"
    printf "    build-relwithdebinfo/  RelWithDebInfo сборки (-O2 -g -fno-omit-frame-pointer)\n"
    printf "    build-release/tests/ или build-debug/tests/       Сборки тестов\n\n"
    
    printf "${YELLOW}ЗАМЕЧАНИЯ ПО САНИТАЙЗЕРАМ:${NC}\n"
    printf "    • Санитайзеры применяются ТОЛЬКО к основному исполняемому файлу\n"
    printf "    • Санитайзеры лучше всего работают с Release сборками (--release)\n"
    printf "    • Статические библиотеки и тесты НЕ используют санитайзеры\n"
    printf "    • ThreadSanitizer конфликтует с AddressSanitizer/LeakSanitizer\n"
    printf "    • MemorySanitizer требует компилятор Clang\n"
    printf "    • Множественные санитайзеры можно комбинировать (кроме thread)\n"
    printf "    • Санитайзеры могут значительно замедлить выполнение\n"
    printf "    • Используйте санитайзеры для отладки и тестирования в продакшене\n\n"
}

# Основная функция сборки
build_target() {
    local target=$1
    local build_dir=$2
    
    print_header "Сборка цели: $target"
    # Очистка директории сборки если запрошено
    clean_build_dir "$build_dir" "$CLEAN_BUILD"
    # Настройка CMake
    print_info "Настройка CMake..."
    configure_cmake_extended "$target" "$build_dir" "$BUILD_TYPE" "$ASTRA_MODE" "$ENABLE_SANITIZERS" "$SANITIZER_TYPES"
    # Сборка
    build_target_cmake "$target" "$build_dir" "$JOBS" "$VERBOSE"
    # Специальная обработка для тестов
    if [[ "$target" == "tests" ]] || [[ "$target" == "all" ]]; then
        if run_tests "$build_dir"; then
            print_success "Тесты прошли успешно"
        else
            print_warning "Некоторые тесты не прошли, но сборка продолжается"
        fi
    fi
    # Создание пакета если запрошено
    if [[ "$target" == "package" ]] || [[ "$target" == "all" ]]; then
        create_package "$build_dir"
    fi
    print_success "Сборка завершена: $target"
}

# Основная функция
main() {
    # Проверяем, что мы в корне проекта
    check_project_root
    
    local target=""
    
    # Парсим аргументы командной строки
    while [[ $# -gt 0 ]]; do
        case $1 in
            --install)
                install_dependencies "${BUILD_DEPS[@]}"
                exit 0
                ;;
            --debug)
                BUILD_TYPE="Debug"
                shift
                ;;
            --release)
                BUILD_TYPE="Release"
                shift
                ;;
            --build-type)
                if [[ -n "${2:-}" ]]; then
                    # нормализуем к нижнему регистру и маппим к валидным значениям CMake
                    case "${2,,}" in
                        debug)
                            BUILD_TYPE="Debug" ;;
                        release)
                            BUILD_TYPE="Release" ;;
                        relwithdebinfo|rel_with_deb_info)
                            BUILD_TYPE="RelWithDebInfo" ;;
                        minsizerel|min_size_rel)
                            BUILD_TYPE="MinSizeRel" ;;
                        *)
                            print_error "Неверный тип сборки для --build-type: $2"
                            print_info "Валидные типы: Debug | Release | RelWithDebInfo | MinSizeRel"
                            exit 1 ;;
                    esac
                    shift 2
                else
                    print_error "Опция --build-type требует значение"
                    print_info "Пример: --build-type RelWithDebInfo"
                    exit 1
                fi
                ;;
            --relwithdebinfo)
                BUILD_TYPE="RelWithDebInfo"
                shift
                ;;
            --device)
                if [[ -n "${2:-}" ]]; then
                    DEVICE_PATH="$2"
                    shift 2
                else
                    print_error "Опция --device требует значение"
                    exit 1
                fi
                ;;
            --astra)
                ASTRA_MODE=true
                shift
                ;;
            --sanitizer)
                if [[ -n "${2:-}" ]] && validate_sanitizers "$2" "${VALID_SANITIZERS[@]}"; then
                    ENABLE_SANITIZERS=true
                    SANITIZER_TYPES="$2"
                    shift 2
                else
                    print_error "Опция --sanitizer требует валидные типы санитайзеров: ${VALID_SANITIZERS[*]}"
                    print_info "Примеры: --sanitizer address, --sanitizer address,leak, --sanitizer thread"
                    exit 1
                fi
                ;;
            --no-sanitizer)
                ENABLE_SANITIZERS=false
                shift
                ;;
            --no-clean)
                CLEAN_BUILD=false
                shift
                ;;
            --verbose)
                VERBOSE=true
                shift
                ;;
            --jobs)
                if [[ -n "${2:-}" ]] && [[ "$2" =~ ^[0-9]+$ ]]; then
                    JOBS="$2"
                    shift 2
                else
                    print_error "Опция --jobs требует положительное целое число"
                    exit 1
                fi
                ;;
            -h|--help)
                usage
                exit 0
                ;;
            -*)
                print_error "Неизвестная опция: $1"
                usage
                exit 1
                ;;
            *)
                if [[ -z "$target" ]]; then
                    target="$1"
                else
                    print_error "Указано несколько целей: $target и $1"
                    exit 1
                fi
                shift
                ;;
        esac
    done
    
    # Цель по умолчанию если не указана
    if [[ -z "$target" ]]; then
        target="all"
    fi
    
    # Валидация типа сборки
    if ! validate_build_type "$BUILD_TYPE" "${VALID_BUILD_TYPES[@]}"; then
        print_error "Неверный тип сборки: $BUILD_TYPE"
        print_info "Валидные типы сборки: ${VALID_BUILD_TYPES[*]}"
        exit 1
    fi
    
    # Валидация цели
    if ! validate_target "$target" "${VALID_TARGETS[@]}"; then
        print_error "Неверная цель: $target"
        print_info "Валидные цели: ${VALID_TARGETS[*]}"
        exit 1
    fi
    
    print_header "Скрипт унифицированной сборки Printcore"
    
    # Получаем директорию сборки
    BUILD_DIR=$(get_build_dir "$BUILD_TYPE")
    
    # Выводим конфигурацию
    print_info "Конфигурация:"
    print_info "  Цель: $target"
    print_info "  Тип сборки: $BUILD_TYPE"
    print_info "  Директория сборки: $BUILD_DIR"
    print_info "  Путь устройства: $DEVICE_PATH"
    print_info "  Режим Astra: $ASTRA_MODE"
    print_info "  Санитайзеры: $(if [[ "$ENABLE_SANITIZERS" == "true" ]]; then echo "Включены ($SANITIZER_TYPES)"; else echo "Отключены"; fi)"
    print_info "  Задачи: $JOBS"
    
    # Собираем цель
    build_target "$target" "$BUILD_DIR"
    
    print_success "Все операции завершены успешно!"
    print_info "Артефакты сборки находятся в: $BUILD_DIR"
    
    # Выводим советы по использованию санитайзеров если включены
    if [[ "$ENABLE_SANITIZERS" == "true" ]]; then
        print_sanitizer_tips "$SANITIZER_TYPES"
    fi
}

# Запускаем основную функцию со всеми аргументами
main "$@" 
