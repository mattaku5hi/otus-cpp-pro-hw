# 13. Компьютерное зрение

### Работа была выполнена на примере двух моделей: логистическая регрессия и TensorFlow (наиболее прецизионная)

## Установка
Для работы с утилитами стоит установить DEB-пакет fashion_mnist:
```bash
sudo dpkg -i ~/Downloads/fashion_mnist-0.0.1-Linux.deb
```

## Логистическая регрессия
Пример запуска:
```bash
fashion_mnist ./12_CV/test.csv ./12_CV/logreg_coef.txt
```

## TensorFlow
Скачать библиотеку TensorFlow для Linux:
```bash
curl -L https://storage.googleapis.com/tensorflow/libtensorflow/libtensorflow-cpu-linux-x86_64-2.15.0.tar.gz -o /tmp/libtf.tar.gz
```
Распаковать библиотеку:
```bash
tar -C /usr/local -xzf /tmp/libtf.tar.gz
sudo ldconfig
```
Далее библиотеку можно подключать через CMakeLists.txt и использовать заголовочные файлы из /usr/local/include/tensorflow в собственных модулях.  
Пример запуска:
```bash
fashion_mnist_tf ./12_CV/test.csv ./12_CV/saved_model
```

## Результаты
Метрики качества точь-в-точь совпадают с ожидаемыми, описанными в условии к домашнему заданию.
