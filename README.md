tenzor-test
===========
<h4>Тестовое задание для компании Tenzor. Универсальный парсер контента, как правило нацеленный на новостные сайты.</h4>

$ gcc -std=c99 -c trim.c
$ g++ trim.o main.cpp -static-libgcc -std=c++11 -lstdc++ -lcurl -lxml2 -O2 -ffast-math -mfpmath=sse -fschedule-insns -fsched-pressure

**Краткое описание.**

За основу для алгоритма был взят популярный способ разметки сайтов: наличие блочного тега, содержимое которого оборачивается в строчные. На практике это выглядит след. образом: блочный тег &lt;div&gt;, строчные тег(и) &lt;p&gt;, &lt;b&gt;, &lt;a&gt; и т.д.
Дополнительным критерием оценки содержимого служит наличие ссылок, однако, вместо подсчета кол-ва ссылок ведется подсчет байт, которые они занимают. Тем самым позволяя обходить ленты новостей вида &lt;a href="link to the page"&gt;Длинное название новости, почти краткое описание&lt;/a&gt;&lt;a ...&gt;Еще одна новость&lt;/a&gt;


**Принцип работы.**

Первым делом инициализируется список блочных/управляющих и строчных/дополнительных тегов. Анализ документа производится с помощью сторонней библиотеки libxml2, которой передается всего три callback'a: начало тега, получение данных, конец тега.
Для определения в любой момент времени имени тега, с которым идет работа, оно помещается в стек(m_levelStack) при входе в тело тега и извлекается при выходе.

Результатом работы программы является список блочных тегов, из которого затем выбирается элемент с наибольшим размером.

Когда обработчику встречается блочный тег, создается соответствующий объект типа Node и помещается в стек для временных объектов (m_parentStack) это позволяет сохранить порядок вложенности тегов, на случай если блочный тег вложен в блочный. 
Далее, при обработке строчных тегов их содержимое добавляется к предыдущему блочному тегу, уникальное поведение создано для ссылок: длина содержимого ссылки записывается в специальное значение m_Links, дабы в дальнейшем вычесть его из общего размера тега и тем самым свести к минимум влияние ссылок на основной контент.
Содержимое тегов не указанных ни в одном из списков игнорируется.
В дополнение к основному фильтру, при добавлении контента происходит проверка глубины вложенности. Так, к примеру, тег оказавшийся слишком глубоко, за чередой неизвестных тегов будет проигнорирован. Подобное поведение присуще разного рода меню и лентам. Уровень вложенности изменяется только тегами, которых нет ни в одном из списков.

После выхода из блочного тега, проверяется размер его содержимого. Пустые блочные теги уничтожаются. Остальные добавляются к результатам выборки.


**Дальнейшее развитие.**

Выделение "словаря" тегов во внешний источник, поддержка дополнительных списков для разных сайтов, имеющих отклонения в разметке.
Проверка типа контента по ссылке, игнорирование изображений, видео и прочего.
Ведение лога своих действий в любой из приемников: master-server, файл, субд.
Исправление допущений в коде, ошибок.
