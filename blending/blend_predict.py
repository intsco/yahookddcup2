# -*- coding: utf-8 -*-

import codecs
import sys

print 'start blend prediction...'

f = open('preds_hash_ranks_{0}_rm.txt'.format(sys.argv[1]), 'r')

coeff = [0.2, 0.1, 0.7]
"""coeff = [0.1555190336016514, #content predict coeff
         -0.4989501463613227,  #item2item predict coeff
         0.8186495743885008] #binsvd predict coeff"""

p_hash = {}

# collect data for all users
for line in f.readlines() :
    list = line.strip().split(';')
    u, i = list[0].split('_')
    r = 1 if int(list[1]) > 0 else -1
    p_list = [float(el[0]) for el in list[2:]]
    
    # predict r
    _r = sum([p_list[ind] * coeff[ind] for ind in range(len(p_list))])
    if (not p_hash.has_key(u)) :
        p_hash[u] = [[i, r, _r]]
    else :
        p_hash[u].append([i, r, _r])
        
# compute error rate
n , err_n = 0, 0
for u, i_list in p_hash.items() :
    sorted_i_list = sorted(i_list, key=lambda el: el[2], reverse=True)
    
    for ind in range(3) :
        if (sorted_i_list[ind][1] < 0) :
            err_n += 1
    for ind in [3, 4, 5] :
        if (sorted_i_list[ind][1] > 0) :
            err_n += 1
    n += 6
    
print 'error rate: {0}'.format(float(err_n) / n * 100)

f.close()