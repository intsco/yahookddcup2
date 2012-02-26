# -*- coding: utf-8 -*-

import codecs

def get_item_freqs(file) :
    item_freqs = {}
    for line in file.readlines() :
        if ('|' not in line) :
            item, r = line.split('\t')
            if (not item_freqs.has_key(item)) :
                item_freqs[item] = 1
            else :
                item_freqs[item] += 1
    return item_freqs
    

fqt = codecs.open("../train_negatives_sample.txt", 'r')
fpy = codecs.open("../_train_negatives_sample_py.txt", 'r')
f = codecs.open("../train_sample.txt", 'r')

qt_items_freqs = get_item_freqs(fqt)
py_items_freqs = get_item_freqs(fpy)
items_freqs = get_item_freqs(f)

qt_items_freqs_sorted = sorted(qt_items_freqs.items(), key=lambda item: item[1], reverse=True)
py_items_freqs_sorted = sorted(py_items_freqs.items(), key=lambda item: item[1], reverse=True)
items_freqs_sorted = sorted(items_freqs.items(), key=lambda item: item[1], reverse=True)

output = codecs.open("draw_neg_compare.csv", 'w')
output.write('{0};{1};{2};{3};{4};{5}\n'.format("qt_neg_set", "qt_count", "py_neg_set", "py_count", "neg_set", "count"))
for i in range(len(items_freqs_sorted)) :
    item = items_freqs_sorted[i][0]
    freq = items_freqs_sorted[i][1]
    
    py_item = py_items_freqs_sorted[i][0] if len(py_items_freqs_sorted) > i else ""
    py_freq = py_items_freqs_sorted[i][1] if len(py_items_freqs_sorted) > i else ""

    qt_item = qt_items_freqs_sorted[i][0] if len(qt_items_freqs_sorted) > i else ""
    qt_freq = qt_items_freqs_sorted[i][1] if len(qt_items_freqs_sorted) > i else ""
    
    output.write('{0};{1};{2};{3};{4};{5}\n'.format(qt_item, qt_freq, py_item, py_freq, item, freq))

output.close()
fqt.close()
fpy.close()
f.close()