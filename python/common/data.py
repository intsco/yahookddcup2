# data.py
# -*- coding: utf-8 -*-

import sys
sys.path.append('../common')
import loader2 as l
cimport numpy as np

ctypedef np.int_t DTYPE_t

cdef class Data(object) :
	cdef char* train_fn = '../train_sample'
	cdef char* valid_fn = '../valid_sample'
	cdef char* tracks_fn = '../_trackData'
	cdef np.ndarray train_users
	cdef np.ndarray train_users_rs
	
	
def class Data2(object) :
	pass
	
def load_data() :
	Data.train_users, Data.train_users_rs = l.load_trainset(Data.train_fn)
	Data2.tracks_tax = l.load_tracks_taxonomy(Data.tracks_fn)
	Data2.valid = l.load_validset(Data.valid_fn)

	
def get_user_vr(user, item) :
	return Data2.valid[user][item]
	
cdef np.ndarray get_train_user_rs(int user) :
	cdef int f = 0
	cdef int l = 0
	f, l = Data.train_users[user]
	return Data.train_users_rs[f:l]

	
def get_valid_items() :
	return Data2.valid.items()
	
def add_valid_r(user, item, r) :
	Data2.valid[user][item] = r


def get_track_alb_art(track) :
	return Data2.tracks_tax.get(track, [-1,-1])[0:2]

def get_track_artist(track) :
	return Data2.tracks_tax[track][1]

def get_track_album(track) :
	return Data2.tracks_tax[track][0]
