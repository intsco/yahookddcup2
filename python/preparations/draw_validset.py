# solution.py
# -*- coding: utf-8 -*-

import random, sys, array
sys.path.append('../common')
from loader import *
import pp

class drawer :
  train_fn = '../trainIdx2_sample'
  #train_fn = '../trainIdx2_firstLines'
  tracks_fn = '../_trackData2'
  validset_fn = '../validIdx2'
  validset_limit_r = 70
  
  items_hi_probs = {}
  users = {}
  users_rs = array.array('I')
  validset = {}
  
  def get_items_hi_probs(self) :  # only for tracks
    print "-> Getting items hi probabilities"
    #self.users, self.users_rs = load_trainset(self.train_fn, self.tracks_fn)
    users_info = load_trainset(self.train_fn, self.tracks_fn)
    self.users, self.users_rs = [el for el in users_info]
    
    j = 0
    n = len(self.users_rs)
    sum_hi_r = 0
    while (j < n) :
      i = self.users_rs[j]
      r = self.users_rs[j+1]
      if (r >= 80) :
        if (not self.items_hi_probs.has_key(i)) :
          self.items_hi_probs[i] = 1
        else :
          self.items_hi_probs[i] += 1
        sum_hi_r += 1
      j += 2
      
    print "items having hi ratings: ", len(self.items_hi_probs)
      
    for item in self.items_hi_probs :
      prob = float(self.items_hi_probs[item]) / sum_hi_r
      if (prob < 0.000000000001) :
        print prob
      self.items_hi_probs[item] = prob
		
    return

  def has_3_hi_ratings(self, first, numb) :
    j = first
    n = first + numb
    hi_r_numb = 0
    #print "f=", first, "n=", n 
    while (j < n) :
			if (self.users_rs[j+1] >= 80) :
				hi_r_numb += 1
			if (hi_r_numb > 2) : return True
			j += 2
    return False

  def get_rand_user_hi_r(self, first, numb) :
		random.seed()
		j = first
		n = first + numb
		hi_r_items = []
		while (j < n) :
			if (self.users_rs[j+1] >= 80) :
				hi_r_items.append(self.users_rs[j])
			j += 2
		rand_ind = random.randint(0, len(hi_r_items)-1)
		return hi_r_items[rand_ind]

  def has_rated(self, first, numb, item) :
		k = first
		while (k < first + numb) :
			if (self.users_rs[k] == item) : 
				return True
			k += 2
		return False

  def get_rand_user_non_r(self, items_hi_probs_list, first, numb) :
		random.seed()
		while (True) :
			hi = len(items_hi_probs_list)
			lo = 0
			x = random.random()
			
			while lo < hi :
				mid = (lo+hi)/2
				midval = items_hi_probs_list[mid][2]
				prev_midval = items_hi_probs_list[mid-1][2]
				#print mid, prev_midval, midval, '\r', 
				if midval < x :
					lo = mid+1
				elif prev_midval > x : 
					hi = mid
				else:
					item = items_hi_probs_list[mid][0]
					break
			
			#print x, mid, items_hi_probs_list[mid-1][2], items_hi_probs_list[mid][2], items_hi_probs_list[mid+1][2]
			#print "i=", item
					
			if (not self.has_rated(first, numb, item)) : 
				return item
	
  def get_user_valid_rs(self, user) :
    first = self.users[user][0]
    numb = self.users[user][1]
    user_rs = {}
    if (self.has_3_hi_ratings(first, numb)) :
      validsetnumb += 6
      for j in range(3) :
        # high rated
        hi_r_item = self.get_rand_user_hi_r(first, numb)
        user_rs[hi_r_item] = 1
        # non rated
        non_r_item = self.get_rand_user_non_r(items_hi_probs_list, first, numb)
        user_rs[non_r_item] = 0
    return user, user_rs
						
  def draw_validset(self) :
    print "-> Drawing valid set"

    # get items hi r probabilities and prob sums
    self.get_items_hi_probs()
    items_hi_probs_list = []
    prob_sum = 0
    for el in self.items_hi_probs.items() :
      tmp_list = [el[0], el[1]]
      prob_sum += el[1]
      tmp_list.append(prob_sum)
      items_hi_probs_list.append(tmp_list)
		
		# prepare job_server
    job_server = pp.Server()
    print "starting pp with", job_server.get_ncpus(), "workers"
        
		# go through all users and get validset (paralleling)
    print "submitting jobs"
    validsetnumb = 0
    users_jobs = []
    # submit jobs
    for user in self.users.keys() :
      job = job_server.submit(self.get_user_valid_rs, (user,), (self.get_rand_user_non_r, self.has_rated, self.get_rand_user_hi_r, self.has_3_hi_ratings), ('random', 'array') )
      users_jobs.append(job)
    
    print "doing jobs"
    # do jobs
    for job in users_jobs :
      user, user_rs = job()
      self.validset[user] = user_rs
      print "u=",user
      
    if (validsetnumb > self.validset_limit_r) : 
      print validsetnumb
      return
				
  def save_to_file(self) :
		print 'Saving validset to text file... ', 
		file = open(self.validset_fn + '.txt', 'w')
		
		for user in self.validset :
			file.write('{0}|{1}\n'.format(user, len(self.validset[user])))
			for item, r in self.validset[user].items() :
				file.write('{0}\t{1}\n'.format(item, r))
				
		print 'OK'
		file.close()  
    
""" MAIN """
def main() :
  drwr = drawer()
  drwr.draw_validset()
  drwr.save_to_file()

main()    
    
    
 