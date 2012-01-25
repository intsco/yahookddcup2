# solution.py
# -*- coding: utf-8 -*-

import time
import random
import sys
import math
import array
import cPickle
sys.path.append('../common')
from loader import *

def load_tracks() :
	f = open('../_trackData2.txt', 'r')
	tracks = {}
	for line in f.readlines() :
		tracks[int(line.split('|')[0])] = 0
	f.close()
	return tracks

def load_users_rs(tracks) :
	fn = 'verify_train_sample_tracks'
	users = {}
	try :
		bin_file = open(fn, 'r')
		users = cPickle.load(bin_file)
		bin_file.close()
		print "	unpickled_users_rs:\t", len(users)
	except IOError as e :    
		f = open('../train_sample.txt', 'r')
		line = f.readline()
		user = -1
		while(line != '') :
			if ('|' in line) :
				user = int(line.strip('\n').split('|')[0])
				users[user] = array.array('I')
			else :
				i = int(line.strip('\r').split('\t')[0])
				r = int(line.strip('\r').split('\t')[1])
				if (tracks.has_key(i)) :
					users[user].append(i)
					users[user].append(r)
			line = f.readline()
			
		print '	loaded users numb:\t', len(users)
		binfile = open(fn, 'w')
		cPickle.dump(users, binfile)
		binfile.close()
	return users

def check_hi_r_items(u_items, hi_r_items) :
	j = 0
	all_hi_r_items = []
	while (j < len(u_items)-1) :
		if (u_items[j+1] >= 80) :
			all_hi_r_items.append(u_items[j])
		j += 2
	for item in hi_r_items :
		if (item not in all_hi_r_items) :
			print '	user {0} bugged!!!'.format(user), hi_r_items
			return False
#	print user, 'verified'
	return True

def calc_testset_items_probs() :
	items = {}
	items_numb = 0
	for line in open('../test.txt').readlines() :
		if ('|' not in line) :
			i = int(line.strip('\n').split('\t')[0])
			r = int(line.strip('\n').split('\t')[1])
			if (r == -1) :
				items_numb += 1
				if (not items.has_key(i)) :
					items[i] = 1
				else :
					items[i] += 1
	for item, numb in items.items() :
		items[item] = float(numb) / items_numb
	print 'test set items probs numb =', len(items)
	return items
	
def calc_items_probs(users) :
	items_numb = 0
	items_probs = {}
	for user, items in users.items() :
		j = 0
		while (j < len(items) - 1) :
			item = items[j]
			r = items[j+1]
			if (r >= 80) :
				items_numb += 1
				if (not items_probs.has_key(item)) :
					items_probs[item] = 1
				else :
					items_probs[item] += 1
			j += 2
			
	for item, numb in items_probs.items() :
		items_probs[item] = float(numb) / items_numb
		
	return items_probs

	
print 'Starting verification ...'

print 'loading tracks'
tracks = load_tracks()
print '	len(tracks) =', len(tracks)

print 'loading users rs'
users = load_users_rs(tracks)

print 'calculation items probs'
items_probs = calc_items_probs(users)
print '	len(items_probs) =', len(items_probs)

items_probs_testset = calc_testset_items_probs()

f = open('../valid_sample.txt', 'r')
lines = f.readlines()
f.close()

print 'verifying validset'
user = -1
hi_r_items = []
items_r_vs = []
items_probs_vs = {}
items_numb_vs = 0
for line in lines :
	if ('|' in line) :
		if (user != -1) :
			if (not check_hi_r_items(users[user], hi_r_items)) :
				break
			if (items_r_vs != 3) :
				print '	bag, user {0} has not 3 hi r items'.format(user)
				break 
		items_r_vs = 0
		hi_r_items = []
		user = int(line.strip('\n').split('|')[0])
	else :
		item = int(line.strip('\n').split('\t')[0])
		r = int(line.strip('\n').split('\t')[1])
		items_r_vs += r
		if (r == 1) :
			hi_r_items.append(item)
		else :
			items_numb_vs += 1
			if (not items_probs_vs.has_key(item)) :
				items_probs_vs[item] = 1
			else :
				items_probs_vs[item] += 1
check_hi_r_items(users[user], hi_r_items)
print '	items_numb_vs', items_numb_vs
print '	items_probs_vs', len(items_probs_vs)

for item, r_numb in items_probs_vs.items() :
	items_probs_vs[item] = float(r_numb) / items_numb_vs

print 'writing items probs'
f2 = open('items_probs_sample.csv', 'w')
f2.write('item;prob;vs_prob;ts_prob\n')
for item, prob in items_probs.items() :
	vs_prob = items_probs_vs.get(item)
	ts_prob = items_probs_testset.get(item)
	f2.write(('{0};{1};{2};{3}\n'.format(item, prob, vs_prob, ts_prob)).replace('.',','))
f2.close()
	
print 'Verification finished'








