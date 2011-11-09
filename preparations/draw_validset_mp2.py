# solution.py
# -*- coding: utf-8 -*-

import time
import random
import sys
import math
import array
sys.path.append('../common')
from loader import *
import multiprocessing as mp
import shmem

def get_items_hi_probs(users_rs, Man) :  # only for tracks
    print "-> Getting items hi rate probs"
    
    items_hi_probs = {}
    j = 0
    n = len(users_rs)
    sum_hi_r = 0
    while (j < n - 1) :
        i = users_rs[j]
        r = users_rs[j+1]
        if (r >= 80) :
            if (not items_hi_probs.has_key(i)) :
                items_hi_probs[i] = 1
            else :
                items_hi_probs[i] += 1
                sum_hi_r += 1
        j += 2

    print "items having hi ratings: ", len(items_hi_probs)

    for item in items_hi_probs :
        prob = float(items_hi_probs[item]) / sum_hi_r
        assert prob > 0.000000000001
        items_hi_probs[item] = prob
  
    prob_sum = 0
    j = 0
    # creating structures in shared memory
    i_hi_p_items = mp.Array('i', [0 for el in range(len(items_hi_probs))], lock=False)
    i_hi_p_probsums = mp.Array('d', [0 for el in range(len(items_hi_probs))], lock=False)
    for item, prob in items_hi_probs.items() :
        prob_sum += prob
        i_hi_p_items[j] = item
        i_hi_p_probsums[j] = prob_sum
        j += 1
    
    return i_hi_p_items, i_hi_p_probsums

def get_rand_user_hi_r(rs) :
    random.seed()
    j = 0
    l = len(rs) - 2
    hi_r_items = []
    while (j <= l) :
        if (rs[j+1] >= 80) :
          hi_r_items.append(rs[j])
        j += 2
    rand_user_hi_r = []
    while (len(rand_user_hi_r) < 3) :
        rand_ind = random.randint(0, len(hi_r_items)-1)
        rand_item = hi_r_items[rand_ind]
        if (rand_item not in rand_user_hi_r) :
            rand_user_hi_r.append(rand_item)
    return rand_user_hi_r

def has_rated(rs, item) :
    k = 0
    while (k <= len(rs)-2) :
        if (rs[k] == item) :
            return True
        k += 2
    return False

def get_rand_user_non_r(rs, Man) :
    random.seed()
    non_r_items = []
    while (len(non_r_items) < 3) :
        hi = len(shmem.data.i_hi_p_probsums)
        lo = 0
        x = random.random()
        item = -1
        while lo < hi :
            mid = (lo+hi)/2
            midval = shmem.data.i_hi_p_probsums[mid]
            prev_midval = shmem.data.i_hi_p_probsums[mid-1]
            #print mid, prev_midval, midval, '\r', 
            if midval < x :
                lo = mid+1
            elif prev_midval > x : 
                hi = mid
            else:
                item = shmem.data.i_hi_p_items[mid]
                break
#        Man['lock'].acqure()
#        print x, mid, i_hi_p_probsums[mid-1], i_hi_p_probsums[mid], i_hi_p_probsums[mid+1]
#        Man['lock'].release()
#        print "i=", item
            
        if (not has_rated(rs, item) and item not in non_r_items) :
            non_r_items.append(item)
    return non_r_items

def has_3_hi_ratings(rs, Man) :
    j = 0
    l = len(rs) - 2
    hi_r_numb = 0
    
#    Man['lock'].acquire()
#    print "f=", j, "l=", l
#    Man['lock'].release()
    
    while (j <= l) :
        if (rs[j+1] >= 80) :
            hi_r_numb += 1
        if (hi_r_numb > 2) : return True
        j += 2
    return False
  
def prime_numb(numb) :
    i = 0
    while (i < numb) :
        j = math.factorial(i)
        i += 1
  
def get_user_valid_rs(user, rs, Man) :
    first = 0
    last = len(rs) - 2 # last item
    user_valid_rs = {} # items which user hi rated or non rated
    
#    Man['lock'].acquire()
#    print 'u=',user, 
#    Man['lock'].release()
    
    if (has_3_hi_ratings(rs, Man)) :
        # high rated
        for hi_r_item in get_rand_user_hi_r(rs) :
            user_valid_rs[hi_r_item] = 1
#            prime_numb(hi_r_item)
        # non rated
        for non_r_item in get_rand_user_non_r(rs, Man) :
            user_valid_rs[non_r_item] = 0
    
    return user, user_valid_rs


def draw_validset(users, users_rs, i_hi_p_items, i_hi_p_probsums, Man) :
    print "-> Drawing valid set"
    
    # preparations
    print "->  praparations for paralleling"
    validsetnumb = 0
    tasks = []
    for user, bounds in users.items() :
        f = bounds[0]
        l = bounds[1] # the last but one element (the last item)
        tasks.append( (get_user_valid_rs, (user, users_rs[f:l+1], Man)) )
        validsetnumb += 6
        if (validsetnumb > Man['validset_limit_r']) :
            break
    
    # parallel computing
    print "->  parallel computing"
    pool = mp.Pool(processes=Man['processes'], initializer=shmem.init, initargs=(i_hi_p_items, i_hi_p_probsums,))
    results = [pool.apply_async(func, args) for func, args in tasks]

    # append users valid rs
    print "->  append users valid rs"
    print 'len(results)=', len(results)
    
    users_validset = {}
    for r in results :
        tmp = r.get()
        if (tmp[1]) :
            users_validset[tmp[0]] = tmp[1]

    return users_validset

      
def save_validset(users_validset, Man) :
    print '-> Saving validset to text file... ' 
    file = open(Man['validset_fn'] + '.txt', 'w')
  
    for user, validset in users_validset.items() :
        file.write('{0}|{1}\n'.format(user, len(validset)))
        for item, r in validset.items() :
            file.write('{0}\t{1}\n'.format(item, r))
    print 'OK'
    file.close()

def save_trainset(users, users_rs, users_validset, Man) :
	print '-> Preparing and saving new trainset to text file... '
	
#	del users
#	del users_rs[:]
	
	users_info = load_trainset(Man['all_train_fn'])
	users, users_rs[:] = [el for el in users_info]
	
	trainset = {}
	for user, bounds in users.items() :
		j, l = bounds[:]
		trainset[user] = array.array('I')
		validset = users_validset.get(user, {})
		val_hir = [item for item, r in validset.items() if r == 1]
		while (j <= l) :
			if (users_rs[j] not in val_hir) :
				trainset[user].append(users_rs[j])
				trainset[user].append(users_rs[j+1])
			j += 2
			
	file = open(Man['train_fn'] + '.txt', 'w')
	for user, train in trainset.items() :
		file.write('{0}|{1}\n'.format(user, len(train)/2))
		j = 0
		while (j < len(train)) :
			item = train[j]
			r = train[j+1]
			file.write('{0}\t{1}\n'.format(item, r))
			j += 2
	file.close()
	print 'OK'

def main() :
	if __name__ == '__main__' :
		Man = {
			'lock' : mp.Manager().Lock(),
			'all_train_fn' : '../all_train_sample',
			'train_fn' : '../train_sample',
			'tracks_fn' : '../_trackData',
			'validset_fn' : '../valid_sample',
			'validset_limit_r' : 70000,
			'processes' : 2}

		# Loading data
		users_info = load_trainset(Man['all_train_fn'], Man['tracks_fn'])
		users, users_rs = [el for el in users_info]

		# Getting items hi rate probs
		i_hi_p_items, i_hi_p_probsums = get_items_hi_probs(users_rs, Man)

		# Draw validset   
		users_validset = draw_validset(users, users_rs, i_hi_p_items, i_hi_p_probsums, Man)

		# Save validset
		save_validset(users_validset, Man)

		# Save new trainset
		save_trainset(users, users_rs, users_validset, Man)

if __name__ == '__main__' :
#    import profile
#    profile.run('main()')
    start = time.time()
    main()
    print time.time() - start
    
    
    
    
    
    
    
    
    
    
    
      
    