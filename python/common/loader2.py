# loader2.py
# -*- coding: utf-8 -*-

import cPickle, array
import sys
import numpy as np

""" Loading data """

def load_tracks_taxonomy(fn) :
	print "-> Loading tracks taxonomy from text file ...", 
	file = open(fn + '.txt', 'r')

	track_tax = {}
	for line in file.readlines() :
		list = line.split('|') # track, album, artist, genres...
		track_tax[int(list[0])] = [int(el.strip('\n')) if el.strip('\n') != 'None' else -1 for el in list[1:] ]

	file.close()
	print "tracks loaded: ", len(track_tax)
	return track_tax

def load_trainset_from_text_file(train_fn, tracks_fn='') :
	print "-> Loading train set from text file ..."

	# calculate data size
	print "-> Calculating data size ..."
	file = open(train_fn + '.txt', 'r')
	line = file.readline()
	users_n = 0
	rs_n = 0
	while(line != '') :
		if (line.find('|') >= 0) :
			users_n += 1
		else :
			rs_n += 1
		line = file.readline()

	users = np.zeros((249012, 2), dtype=np.int32)
	users_rs = np.zeros((rs_n, 2), dtype=np.int32)

	file.close()

	if (tracks_fn != '') :
		track_tax = load_tracks_taxonomy(tracks_fn)
	
	# parse file
	print "-> Parsing text file ..."
	file = open(train_fn + '.txt', 'r')
	t_rs_numb = 0
	prev_t_rs_numb = 0
	user, r_numb = [int(el) for el in file.readline().strip('\n').split('|')]
	line = file.readline()
	while(line != '') :
		if (line.find('|') >= 0) :
			users[user] = [prev_t_rs_numb, t_rs_numb - 1]
			prev_t_rs_numb = t_rs_numb			
			user, r_numb = [int(el) for el in line.strip('\n').split('|')]
		else :
			item, r = [int(el) for el in line.strip('\n').split('\t')]
			if (tracks_fn != '') :
				if (track_tax.has_key(item)) :
					users_rs[t_rs_numb] = item, r
#					users_rs[t_rs_numb2+1] = r
					t_rs_numb += 1
			else :
				users_rs[t_rs_numb] = item, r
#				users_rs[t_rs_numb2+1] = r
				t_rs_numb += 1
		line = file.readline()
	users[user] = [prev_t_rs_numb, t_rs_numb - 1]
  
	# save data to binfile
	print "-> Saving data to binfile ..."
	if (tracks_fn == '') :
		np.save(train_fn + '_1', users)
		np.save(train_fn + '_2', users_rs)
	else :
		np.save(train_fn + '_tracks_1', users)
		np.save(train_fn + '_tracks_2', users_rs)

	print "users_rs numb: ", len(users_rs)
	print "users numb: ", len(users)

	file.close()
	return users, users_rs

def load_trainset(fn, tracks_fn='') :
	print "-> Loading train set... " 
      
	try :
		if (tracks_fn == '') :
			users = np.load(fn + '_1.npy')
			users_rs = np.load(fn + '_2.npy')
		else :
			users = np.load(fn + '_tracks_1.npy')
			users_rs = np.load(fn + '_tracks_2.npy')
		print "unpickled users, users_rs:\t", len(users), len(users_rs)
	except IOError:
		users, users_rs = load_trainset_from_text_file(fn, tracks_fn)
  
	return users, users_rs
  
def load_validset(fn) :
  print "-> Loading valid/test set from text file '{0}'... ".format(fn), 
  testset = {}
  
  test_file = open(fn + '.txt', 'r')
  lines = test_file.readlines()
  
  for line in lines :
    if (line.find('|') >= 0) :
      user = int(line.split('|')[0])
      if (not testset.has_key(user)) :
        testset[user] = {}
    else :
      item, r = [int(elem) for elem in line.strip('\n').split('\t')]
      testset[user][item] = -10
  
  test_file.close()
  print "testset len", len(testset)
  
  return testset

#if __name__ == '__main__' :
#	users, users_rs = load_trainset('../train_sample')
