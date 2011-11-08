# -*- coding: utf-8 -*-

import random, sys, math, array
sys.path.append('../common')
from loader import *

class Content_pred() :	
	# public
	def study(self, train, valid) :
		print '->	studying: prapare tracks taxonomy'
		
		self.tracks = load_tracks_taxonomy('../_trackData')
		self.train = train
		self.valid = valid
		
		print '->	OK'
	
	# public
	def predict(self) :
		print "->	predicting: content-based prediction... "
		
		for user, rs in self.valid.items() :
			for item, r in rs.items() :
				a = self.tracks[item][1] # artist
				b = self.tracks[item][0] # album
				ra = self.get_user_r(user, a)
				rb = self.get_user_r(user, b)
				a_avgr, b_avgr = self.get_alb_art_tracks_user_avgr(user, a, b)
#				print 'user, item, a, ra, b, rb:', user, item, a, ra, b, rb
				self.valid[user][item] = ra + rb + a_avgr + b_avgr
			
		self.classify()	
		print "->	users predicted: ", len(self.valid)
		
		return self.valid
	
	def get_alb_art_tracks_user_avgr(self, user, artist, album) :
		if (artist >= 0) :
			j = 0
			art_rsum = 0
			art_rnumb = 0
			alb_rsum = 0
			alb_rnumb = 0
			user_rs = self.train[user]
			while (j < len(user_rs)) :
				item = user_rs[j]
#				print self.tracks.get(item, [-1, -1])[0:2]
				alb, art = self.tracks.get(item, [-1, -1])[0:2]
				
				if (art == artist) :
					art_rnumb += 1
					art_rsum += user_rs[j+1]
				if (alb == album) :
					alb_rnumb += 1
					alb_rsum += user_rs[j+1]
				j += 2
			art_avgr = 0
			if (art_rnumb > 0) : 
				art_avgr = float(art_rsum) / art_rnumb
			alb_avgr = 0
			if (alb_rnumb > 0) : 
				alb_avgr = float(alb_rsum) / alb_rnumb
			return alb_avgr, art_avgr
		return 0, 0
	
	def get_user_r(self, user, item) :
		if (item >= 0) :
			j = 0
			user_rs = self.train[user]
			while (j < len(user_rs)) :
				if (user_rs[j] == item) :
					return user_rs[j+1]
				j += 2
		return 0

	# divide ratings into 2 groups
	def classify(self) : 
		print "->		predicting: classification"
		
		for user, rs in self.valid.items() :
			user_rs = sorted(rs.items(), key=lambda item: item[1], reverse=True)
			for j in range(6) :
				item = user_rs[j][0]
				if (j < 3) : 
					self.valid[user][item] = 1
				else :
					self.valid[user][item] = 0
				
		print '->		OK'
  
  
  
  
  
  
  
  
  
  
  
  
  
  