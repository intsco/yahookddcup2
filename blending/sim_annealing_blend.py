# -*- coding: utf-8 -*-

import codecs
import sys
import random as rnd


# compute error rate
def get_err(p_hash, coeff) :
    n , err_n = 0, 0
    
    # compute pred rating
    pred_rs = {}
    for u, i_list in p_hash.items() :
        pred_r = [coeff[ind] * i_list[2][ind] for ind in range(3)]
        if (not pred_rs.has_key(u)) :
            pred_rs[u] = [[u, i_list[0], i_list[1], pred_r]]
        else :
            pred_rs[u].append([u, i_list[0], i_list[1], pred_r])
    
    # sort pred ratings
    for u, i_list in pred_rs.items() :
        sorted_list = sorted(i_list, key=lambda el: el[3], reverse=True)
        
        for ind in range(3) :
            if (sorted_i_list[ind][2] < 0) :
                err_n += 1
        for ind in [3, 4, 5] :
            if (sorted_i_list[ind][2] > 0) :
                err_n += 1
    
    n = len(pred_rs) * 6
        
    return float(err_n) / n * 100

def random_modifi(coeff, mr) :
    return [c * ((rnd.random()*2 - 1) * mr) for c in coeff]


print 'Start simulated annealing blending...'

pf = open('preds_hash_ranks_{0}_rm.txt'.format(sys.argv[1]), 'r')

p_hash = {}

# collect data for all users
for line in f.readlines() :
    list = line.strip().split(';')
    u, i = list[0].split('_')
    r = 1 if int(list[1]) > 0 else -1
    p_list = [float(el[0]) for el in list[2:]]

    if (not p_hash.has_key(u)) :
        p_hash[u] = [[i, r, [p_list]]]
    else :
        p_hash[u].append([i, r, [p_list]])
        
# start simulated annealing
T = 1000
eps = 0.01
mr = 0.2
coeff = [0, 0, 0]

while (T > eps) :
    coeff

    
print 'Error rate: {0}'.format(0)

pf.close()