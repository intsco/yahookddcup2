# -*- coding: utf-8 -*-

import codecs


def get_item_freqs(lines) :
    item_freqs = {}
    for line in lines :
        if ('|' not in line) :
            item, r = line.split('\t')
            if (not item_freqs.has_key(item)) :
                item_freqs[item] = 1
            else :
                item_freqs[item] += 1
    return item_freqs
    
def filter_hi_rs(lines) :
    new_lines = []
    for line in lines :
        if ('|' not in line) :
            item, r = line.split('\t')
            if (int(r) >= 80) :
                new_lines.append(line)
    return new_lines

def sort_items(hash) :
    return sorted(hash.items(), key=lambda item: item[1], reverse=True)

def append_items_numbs(items_numbs, items_freqs, ind) :
    for item, numb in items_freqs.items() :
        if (items_numbs.has_key(item)) :
            items_numbs[item][ind] = numb

def load_track_tax() :
    tracks = {}
    for line in codecs.open("../_trackData.txt", 'r').readlines() :
        tracks[line.split('|')[0]] = 1
    return tracks

fqt = codecs.open("../train_negatives_sample_qt.txt", 'r')
fpy = codecs.open("../train_negatives_sample_py.txt", 'r')
f = codecs.open("../all_train.txt", 'r')
ft = codecs.open("../test.txt", 'r')

qt_items_freqs = get_item_freqs(fqt.readlines())
py_items_freqs = get_item_freqs(fpy.readlines())
items_freqs_train = get_item_freqs(filter_hi_rs(f.readlines()))
items_freqs_test = get_item_freqs(filter_hi_rs(ft.readlines()))

"""qt_items_freqs_sorted = sort_items(qt_items_freqs)
py_items_freqs_sorted = sort_items(py_items_freqs)
items_freqs_train_sorted = sort_items(items_freqs_train)
items_freqs_test_sorted = sort_items(items_freqs_test)"""

items_numbs = {}
for item, numb in items_freqs_train.items() :
    items_numbs[item] = [0, numb, 0, 0, 0]
append_items_numbs(items_numbs, items_freqs_test, 0)
append_items_numbs(items_numbs, qt_items_freqs, 2)
append_items_numbs(items_numbs, py_items_freqs, 3)
        
tracks = load_track_tax()
for item, numbs in items_numbs :
    if (tracks.has_key(item)) :
        numbs[4] = 1

output = codecs.open("draw_neg_compare.csv", 'w')
output.write('item;test_numb;train_sample_numb;qt_neg_sample_numb;py_neg_sample_numb;is_track\n')
"""output.write('{0};{1};{2};{3};{4};{5};{6};{7}\n'.
             format("qt_neg_set", "qt_count", "py_neg_set", "py_count", 
                    "neg_train_set", "train_count", "neg_test_set", "test_count"))"""
"""for i in range(len(items_freqs_test_sorted)) :
    item_train = items_freqs_test_sorted[i][0]
    freq_train = items_freqs_test_sorted[i][1]
    
    item_test = items_freqs_test_sorted[i][0] if len(items_freqs_test_sorted) > i else ""
    freq_test = items_freqs_test_sorted[i][1] if len(items_freqs_test_sorted) > i else ""
    
    py_item = py_items_freqs_sorted[i][0] if len(py_items_freqs_sorted) > i else ""
    py_freq = py_items_freqs_sorted[i][1] if len(py_items_freqs_sorted) > i else ""

    qt_item = qt_items_freqs_sorted[i][0] if len(qt_items_freqs_sorted) > i else ""
    qt_freq = qt_items_freqs_sorted[i][1] if len(qt_items_freqs_sorted) > i else "" 
    
    output.write('{0};{1};{2};{3};{4};{5};{6};{7}\n'.
                 format(qt_item, qt_freq, py_item, py_freq, 
                        item_train, freq_train, item_test, freq_test))"""

for item, numbs in items_numbs.items() :
    output.write('{0};{1};{2};{3};{4};{5}\n'.
                 format(item, numbs[0], numbs[1], numbs[2], numbs[3], numbs[4]))

output.close()
fqt.close()
fpy.close()
f.close()
ft.close()