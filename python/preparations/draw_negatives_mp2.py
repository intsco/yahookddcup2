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

	print "prob_sum =", prob_sum
	return i_hi_p_items, i_hi_p_probsums

def has_rated(rs, item) :
	k = 0
	while (k <= len(rs)-2) :
		if (rs[k] == item) :
			return True
		k += 2
	return False

def get_rand_user_negatives(rs, Man) :
	non_r_items = []
	while (len(non_r_items) < len(rs)) :
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
  
def get_user_nagatives(user, rs, Man) :
	first = 0
	last = len(rs) - 2 # last item
	user_negatives = [] # items which user did not rated

	#    Man['lock'].acquire()
	#    print 'u=',user, 
	#    Man['lock'].release()    

	# get user negative items
	for non_r_item in get_rand_user_negatives(rs, Man) :
		user_negatives.append(non_r_item)

	return user, user_negatives


def draw_negatives(users, users_rs, i_hi_p_items, i_hi_p_probsums, Man) :
	print "-> Drawing negatives..."

	# preparations
	print "->  praparations for paralleling"
	validsetnumb = 0
	tasks = []
	for user, bounds in users.items() :
		f = bounds[0]
		l = bounds[1] # the last but one element (the last item)
		tasks.append( (get_user_nagatives, (user, users_rs[f:l+1], Man)) )

	# parallel computing
	print "->  parallel computing"
	pool = mp.Pool(processes=Man['processes'], initializer=shmem.init, initargs=(i_hi_p_items, i_hi_p_probsums,))
	results = [pool.apply_async(func, args) for func, args in tasks]

	# append users negatives
	print "->  append users negatives"
	print 'len(results)=', len(results)

	users_negatives = {}
	users_n = len(users)
	i = 0.0
	for r in results :
		tmp = r.get()
		users_negatives[tmp[0]] = tmp[1]
		i += 1.0
		print "{0} user(s) and {1} % completed\r".format(i, i / users_n * 100),
		
	print '\nlen(users_negatives)=', len(users_negatives)
	return users_negatives

      
def save_negatives(users_negatives, Man) :
	print '-> Saving negatives to text file... ' 
	file = open(Man['train_negatives_fn'] + '.txt', 'w')

	for user, negatives in users_negatives.items() :
		file.write('{0}|{1}\n'.format(user, len(negatives)))
		for item in negatives :
			file.write('{0}\t{1}\n'.format(item, 0))
	print 'OK'
	file.close()

def main() :
	if __name__ == '__main__' :
		Man = {
			'lock' : mp.Manager().Lock(),
			'train_fn' : '../train_sample',
			'train_negatives_fn' : '../train_negatives_sample',
			'processes' : 2}

		# Loading data
		users_info = load_trainset(Man['train_fn'], '')
		users, users_rs = [el for el in users_info]

		# Getting items hi rating probs
		i_hi_p_items, i_hi_p_probsums = get_items_hi_probs(users_rs, Man)

		# Draw negative items   
		users_negatives = draw_negatives(users, users_rs, i_hi_p_items, i_hi_p_probsums, Man)

		# Save negatives
		save_negatives(users_negatives, Man)

if __name__ == '__main__' :
#    import profile
#    profile.run('main()')
    start = time.time()
    main()
    print time.time() - start
    
    
    
    
    
    
    
    
    
    
    
      
    