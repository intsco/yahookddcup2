# -*- coding: utf-8 -*-

import codecs
import sys

def append_preds(p_hash, ind, pfile) :
    for line in pfile.readlines() :
        u, i, r = line.strip().split(';')
        u_i = '{0}_{1}'.format(u, i)
        p_hash[u_i][ind] = float(r)

print 'start'

fplist = [open('ranks_content_pred_{0}_def=22.txt'.format(sys.argv[2]), 'r'),
          open('ranks_itemnn_pred_{0}_n=100_gama=1.7.txt'.format(sys.argv[2]), 'r'),
          open('ranks_binsvd_pred_{0}_f=2000_st=1000.txt'.format(sys.argv[2]))]

v_fn = 'ranks_{0}.txt'.format(sys.argv[2])
fv = open(v_fn, 'r')

p_hash = {}
for linev in fv.readlines() :
    u, i, r = linev.strip().split(';')
    u_i = '{0}_{1}'.format(u, i)
    p_list = []
    p_list.append(int(r.split('.')[0]) )
    p_list += [0 for f in fplist]
    p_hash[u_i] = p_list
    
print len(p_hash)
    
for ind in range(len(fplist)) :
    append_preds(p_hash, ind+1, fplist[ind])
    print len(p_hash)

for file in fplist :
    file.close()
fv.close()

fn = 'preds_hash_{0}_{1}.txt'.format(v_fn.split('.')[0], sys.argv[1])
if (sys.argv[1] == 'rm') :
    hf = open(fn, 'w')
    for u_i, rs in p_hash.items() :
        hf.write('{0};{1}\n'.format(u_i, ';'.join(['{0}'.format(el) for el in rs])) )
    hf.close()
else :
    hf = open(fn, 'w')
    for u_i, rs in p_hash.items() :
        hf.write('{0}'.format(-1 if rs[0] < 0 else 1))
        ind = 1
        for r in rs[1:] :
            hf.write('\t{0}:{1}'.format(ind, rs[ind]/6) )
            ind += 1
        hf.write('\n')
    hf.close()

print 'finished'
