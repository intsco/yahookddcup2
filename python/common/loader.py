# solution.py
# -*- coding: utf-8 -*-

import random, sys, math, cPickle, array

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
	print "-> Loading train set from text file ...", 

	users = {}
	users_rs = array.array('I')

	if (tracks_fn != '') :
		track_tax = load_tracks_taxonomy(tracks_fn)
		#tracks = track_tax.keys()

	file = open(train_fn + '.txt', 'r')
		
	user = -1
	t_rs_numb2 = 0
	prev_t_rs_numb2 = -1
	line = file.readline()
	while(line != '') :
		if (line.find('|') >= 0) :
			users[user] = [prev_t_rs_numb2, t_rs_numb2 - 1]
			prev_t_rs_numb2 = t_rs_numb2			
			user, r_numb = [int(el) for el in line.strip('\n').split('|')]
		else :
			item, r = [int(el) for el in line.strip('\n').split('\t')]
			if (tracks_fn != '') :
				if (track_tax.has_key(item)) :
					t_rs_numb2 += 2
					users_rs.append(item)
					users_rs.append(r)
			else :
				t_rs_numb2 += 2
				users_rs.append(item)
				users_rs.append(r)
		line = file.readline()
	users[user] = [prev_t_rs_numb2, t_rs_numb2 - 1]
	del users[-1]
  
	if (tracks_fn == '') :
		binfile = open(train_fn, 'w')
	else :
		binfile = open(train_fn + '_tracks', 'w')
	users_info = (users, users_rs)
	cPickle.dump(users_info, binfile)
	binfile.close()
			
	print "users_rs numb: ", len(users_rs)
	print "users numb: ", len(users)
		 
	file.close()
	return users, users_rs

def load_trainset(fn, tracks_fn='') :
  print "-> Loading train set... ", 
  
  users = {}
  users_rs = array.array('I')
  
  try :
    if (tracks_fn == '') :
      bin_file = open(fn, 'r')
    else :
      bin_file = open(fn + '_tracks', 'r')
    #users, users_rs = cPickle.load(bin_file)
    users_info = cPickle.load(bin_file)
    users, users_rs = [el for el in users_info]
    bin_file.close()
    print "unpickled_users_rs:\t", len(users_rs)
    #print users[24530], users_rs[users[24530][0]], users_rs[users[24530][0] + 1]
  except IOError as e :
    users, users_rs = load_trainset_from_text_file(fn, tracks_fn)
  
  return users, users_rs

def load_trainset2(fn, tracks_fn='') :
	print '-> Loading transet2 ...'
	users, users_rs = load_trainset(fn, tracks_fn)
	
	print '->	transform trainset... '
	trainset = {}
	for user, bounds in users.items() :
		trainset[user] = array.array('I')
		j, l = bounds
		while (j <= l) :
			trainset[user].append(users_rs[j])
			trainset[user].append(users_rs[j+1])
			j += 2
	print '->	trainset len = ', len(trainset)
	return trainset

  
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
