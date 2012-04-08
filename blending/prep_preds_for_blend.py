# -*- coding: utf-8 -*-

import codecs

print 'start'

#fn = 'content_pred_valid_def=22.txt'
#fn = 'itemnn_pred_valid_n=100_gama=1.7.txt'
#fn = 'binsvd_pred_valid_f=2000_st=1000.txt'
#fn = 'valid.txt'

flist = ['content_pred_test_def=22.txt',
         'itemnn_pred_test_n=100_gama=1.7.txt',
         'binsvd_pred_test_f=2000_st=1000.txt',
         'test.txt']

for fn in flist :

    if (fn == 'valid.txt' or fn == 'test.txt') :
        set_ranks = False
    else : set_ranks = True
    
    inf = codecs.open(fn, 'r', 'utf-8')
    outf = codecs.open('ranks_' + fn, 'w', 'utf-8')
    
    def sort_and_flush(u, buff, set_ranks) :
        sbuff = sorted(buff, key=lambda row: row[1])
    
        if (set_ranks) :
            rank = 1
            for line in sbuff :
                line[1] = rank
                rank += 1
    
        for line in sbuff :
            outf.write("{0};{1};{2}\n".format(u, line[0], line[1]))
    
    buff = []
    ind, n, u = 0, 0, 0
    for line in inf.readlines() :
        if (ind >= 6) :
            sort_and_flush(u, buff, set_ranks)
            buff = []
            ind = 0
        
        if ('|' in line) :
            u = int(line.split('|')[0])
            continue
            
        ind += 1
        i, r = line.strip('\r\n').split('\t')
        buff.append([int(i), float(r)])
        if n % 100000 == 0 :
            print "{0} lines processed".format(n)
        n += 1
    sort_and_flush(u, buff, set_ranks)
    
    
    inf.close()
    outf.close()

print 'Finished'