![Task](https://raw.githubusercontent.com/funbotan/MIET-IPS/master/individual/task.svg)

Точное значение интеграла равно 2+π, т.е. 5.14159...

Обзор инструментов параллелизации:

* Intel Cilk Plus — расширение языка С++, призванное упростить написание многопоточных программ. Cilk Plus представляет собой динамический планировщик исполнения потоков и набор ключевых слов, сообщающих компилятору о возможности применения той или иной схемы планирования.
* Intel Parallel Inspector — инструмент для обнаружения ошибок памяти и потоков в последовательных и параллельных приложениях на платформах Windows и Linux.
* Intel VTune Amplifier — средство для оптимизации производительности и профилировки параллельных приложений.

Для вычисления будем использовать метод трапеций.

Проверим корректность реализации последовательного и параллельного методов вычисления:

```
Serial trapeze = 5.14159
Parallel trapeze = 5.14159
```

Сравним скорость:

```
Serial duration: 33206934 microseconds
Parallel duration: 19846215 microseconds
Boost ratio: 1.67321
```

Intel Parallel Inspector:

![Inspector](https://raw.githubusercontent.com/funbotan/MIET-IPS/master/individual/inspector.jpg)

Inspector находит проблему, однако, мой код не является ее источником.

Intel VTune Amplifier:

![Amplifier](https://raw.githubusercontent.com/funbotan/MIET-IPS/master/individual/amplifier.png)

По результатам анализа реализацию метода можно считать эффективной.