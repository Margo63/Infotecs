# Infotecs
## Инструкция по сборке и запуску

```bash
# Создание директории для сборки
mkdir build && cd build

# Генерация файлов сборки
cmake ..

# Сборка проекта
make

# Запуск приложения (из директории build)
./console/app ../test.log INFO