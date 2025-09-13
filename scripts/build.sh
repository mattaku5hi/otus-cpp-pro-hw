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
BUILD_PACKAGE=true  # Прибиваем гвоздями

# Управление тестами (по умолчанию запускается GTest)
ENABLE_BOOST="OFF"
ENABLE_GTEST="ON"

# Доступные типы сборки
VALID_BUILD_TYPES=("Debug" "Release" "RelWithDebInfo" "MinSizeRel")

# Список зависимостей для установки
BUILD_DEPS=(
    build-essential
    cmake
    libgtest-dev
    libgmock-dev
    libboost-test-dev
)

# Информация об использовании
usage() {
    printf "${CYAN}Унифицированный скрипт сборки для проекта Printcore${NC}\n\n"
    
    printf "${YELLOW}ИСПОЛЬЗОВАНИЕ:${NC}\n"
    printf "    %s [OPTIONS]\n\n" "$(basename "$0")"
    
    printf "${YELLOW}ДЕЙСТВИЕ:${NC}\n"
    printf "    Собирает проект и запускает выбранные юнит-тесты (по умолчанию: GTest).\n\n"
    
    printf "${YELLOW}ОПЦИИ:${NC}\n"
    printf "    ${GREEN}--install${NC}              Установка необходимых зависимостей (требует sudo)\n"
    printf "    ${GREEN}--build-type TYPE${NC}      Тип сборки: Debug | Release | RelWithDebInfo | MinSizeRel (по умолчанию: Release)\n"
    printf "    ${GREEN}--release${NC}              Тип сборки: Release (по умолчанию)\n"
    printf "    ${GREEN}--debug${NC}                Тип сборки: Debug\n"
    printf "    ${GREEN}--relwithdebinfo${NC}       Тип сборки: RelWithDebInfo\n"
    printf "    ${GREEN}--minsize${NC}              Тип сборки: MinSizeRel\n"
    printf "    ${GREEN}--no-clean${NC}             Не очищать директорию сборки перед сборкой\n"
    printf "    ${GREEN}--verbose${NC}              Включение подробного вывода сборки\n"
    printf "    ${GREEN}--jobs N${NC}               Количество параллельных задач сборки (по умолчанию: %d)\n" "$(get_cpu_cores)"
    printf "    ${GREEN}--tests TYPE${NC}           Какие тесты собирать/запускать: boost | gtest | all (по умолчанию: gtest)\n"
    printf "    ${GREEN}-h, --help${NC}             Показать это сообщение помощи\n\n"
    
    printf "${YELLOW}ПРИМЕРЫ:${NC}\n"
    printf "    %s static-lib                    # Сборка статической библиотеки\n" "$(basename "$0")"
    printf "    %s --build-type Debug executable # Сборка исполняемого файла в режиме Debug\n" "$(basename "$0")"
    printf "    %s --build-type RelWithDebInfo executable # Сборка с RelWithDebInfo (-O2 -g -fno-omit-frame-pointer)\n" "$(basename "$0")"
    printf "    %s --build-type Release --sanitizer address executable # Сборка исполняемого файла с AddressSanitizer\n" "$(basename "$0")"
    printf "    %s --build-type RelWithDebInfo executable      # Сборка исполняемого файла с RelWithDebInfo (-O2 -g -fno-omit-frame-pointer)\n" "$(basename "$0")"
    printf "    %s --build-type Release --sanitizer address,leak executable # Сборка с Address+LeakSanitizer\n" "$(basename "$0")"
    printf "    %s --build-type Release --sanitizer thread executable # Сборка исполняемого файла с ThreadSanitizer\n" "$(basename "$0")"
    printf "    %s --install                     # Только установка зависимостей\n" "$(basename "$0")"
    printf "    %s tests                         # Сборка и запуск тестов\n\n" "$(basename "$0")"
    
    printf "${YELLOW}ДИРЕКТОРИИ СБОРКИ:${NC}\n"
    printf "    build-release/     Release сборки\n"
    printf "    build-debug/       Debug сборки\n"
    printf "    build-relwithdebinfo/  RelWithDebInfo сборки\n"
    printf "    build-minsizerel/      MinSizeRel сборки\n\n"
}

# Основная функция сборки
build_target() {
    local build_dir=$1
    
    print_header "Сборка проекта"
    # Очистка директории сборки если запрошено
    clean_build_dir "$build_dir" "$CLEAN_BUILD"

    # Простая конфигурация CMake под этот репозиторий
    print_info "Настройка CMake..."
    local cmake_args=(
        -S .
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
        -DENABLE_BOOST_TEST="$ENABLE_BOOST"
        -DENABLE_GOOGLE_TEST="$ENABLE_GTEST"
    )
    configure_cmake "$build_dir" "${cmake_args[@]}"

    # Сборка
    build_target_cmake "" "$build_dir" "$JOBS" "$VERBOSE"

    # Запуск тестов по умолчанию, если они были включены
    if [[ "$ENABLE_BOOST" == "ON" || "$ENABLE_GTEST" == "ON" ]]; then
        print_info "Запуск тестов..."
        if ctest --test-dir "$build_dir" --output-on-failure; then
            print_success "Тесты прошли успешно"
        else
            print_error "Некоторые тесты не прошли"; exit 2
        fi
    else
        print_info "Тесты отключены (--tests не указан или равен none)"
    fi

    # Создание DEB пакета, если запрошено
    if [[ "$BUILD_PACKAGE" == "true" ]]; then
        print_info "Сборка DEB пакета через CPack..."
        if cmake --build "$build_dir" --target package; then
            print_success "DEB пакет успешно собран (см. артефакты в $build_dir)"
        else
            print_error "Не удалось собрать DEB пакет"; exit 3
        fi
    fi

    print_success "Сборка завершена"
}

# Основная функция
main() {
    # Проверяем, что мы в корне проекта
    check_project_root
    
    # без целей; только опции
    
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
            --tests)
                if [[ -n "${2:-}" ]]; then
                    case "${2,,}" in
                        boost)
                            ENABLE_BOOST="ON"; ENABLE_GTEST="OFF" ;;
                        gtest)
                            ENABLE_BOOST="OFF"; ENABLE_GTEST="ON" ;;
                        all)
                            ENABLE_BOOST="ON"; ENABLE_GTEST="ON" ;;
                        *)
                            print_error "Опция --tests требует значение: boost | gtest | all"; exit 1 ;;
                    esac
                    shift 2
                else
                    print_error "Опция --tests требует значение"; exit 1
                fi
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
            *)
                print_error "Неизвестная опция: $1"; usage; exit 1 ;;
        esac
    done
    
    # Валидация типа сборки
    if ! validate_build_type "$BUILD_TYPE" "${VALID_BUILD_TYPES[@]}"; then
        print_error "Неверный тип сборки: $BUILD_TYPE"
        print_info "Валидные типы сборки: ${VALID_BUILD_TYPES[*]}"
        exit 1
    fi
    
    print_header "Скрипт сборки"
    
    # Получаем директорию сборки
    BUILD_DIR=$(get_build_dir "$BUILD_TYPE")
    
    # Выводим конфигурацию
    print_info "Конфигурация:"
    print_info "  Тип сборки: $BUILD_TYPE"
    print_info "  Директория сборки: $BUILD_DIR"
    print_info "  Задачи: $JOBS"
    
    # Собираем
    build_target "$BUILD_DIR"
    
    print_success "Все операции завершены успешно!"
    print_info "Артефакты сборки находятся в: $BUILD_DIR"
}

# Запускаем основную функцию со всеми аргументами
main "$@" 
