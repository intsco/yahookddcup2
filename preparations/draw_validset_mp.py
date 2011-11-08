# solution.py
# -*- coding: utf-8 -*-

import random, sys, array
sys.path.append('../common')
from loader import *
import pp

class Drawer :
  train_fn = '../trainIdx2_sample'
  #train_fn = '../trainIdx2_firstLines'
  tracks_fn = '../_trackData2'
  validset_fn = '../validIdx2'
  validset_limit_r = 70
  
  items_hi_probs = {}
  users = {}
  users_rs = array.array('I')
  validset = {}
  
def get_items_hi_probs(dr) :  # only for tracks
  print "-> Getting items hi probabilities"
  #drawer.users, drawer.users_rs = load_trainset(drawer.train_fn, drawer.tracks_fn)
  users_info = load_trainset(dr.train_fn, dr.tracks_fn)
  dr.users, dr.users_rs = [el for el in users_info]
  
  j = 0
  n = len(dr.users_rs)
  sum_hi_r = 0
  while (j < n) :
    i = dr.users_rs[j]
    r = dr.users_rs[j+1]
    if (r >= 80) :
      if (not dr.items_hi_probs.has_key(i)) :
        dr.items_hi_probs[i] = 1
      else :
        dr.items_hi_probs[i] += 1
      sum_hi_r += 1
    j += 2
    
  print "items having hi ratings: ", len(dr.items_hi_probs)
  print "users: ", len(dr.users)
  print "users_rs: ", len(dr.users_rs)
    
  for item in dr.items_hi_probs :
    prob = float(dr.items_hi_probs[item]) / sum_hi_r
    if (prob < 0.000000000001) :
      print prob
    dr.items_hi_probs[item] = prob
  
  return

def get_rand_user_hi_r(dr, first, numb) :
  random.seed()
  j = first
  n = first + numb
  hi_r_items = []
  while (j < n) :
    if (dr.users_rs[j+1] >= 80) :
      hi_r_items.append(dr.users_rs[j])
    j += 2
  rand_ind = random.randint(0, len(hi_r_items)-1)
  return hi_r_items[rand_ind]

def has_rated(dr, first, numb, item) :
  k = first
  while (k < first + numb) :
    if (dr.users_rs[k] == item) : 
      return True
    k += 2
  return False

def get_rand_user_non_r(dr, items_hi_probs_list, first, numb) :
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
        
    if (not has_rated(dr, first, numb, item)) : 
      return item

def has_3_hi_ratings(dr, first, numb) :
  j = first
  n = first + numb
  hi_r_numb = 0
  #print "f=", first, "n=", n 
  while (j < n) :
    if (dr.users_rs[j+1] >= 80) :
      hi_r_numb += 1
    if (hi_r_numb > 2) : return True
    j += 2
  return False

def get_user_valid_rs(dr, user, items_hi_probs_list) :
  print len(dr.users)
  print len(dr.users_rs)
  
  first = dr.users[user][0]
  numb = dr.users[user][1]
  user_rs = {}
  if (has_3_hi_ratings(dr, first, numb)) :
    for j in range(3) :
      # high rated
      hi_r_item = get_rand_user_hi_r(dr, first, numb)
      user_rs[hi_r_item] = 1
      # non rated
      non_r_item = get_rand_user_non_r(dr, items_hi_probs_list, first, numb)
      user_rs[non_r_item] = 0
  return (user, user_rs)
          
def draw_validset(dr) :
  print "-> Drawing valid set"

  # get items hi r probabilities and prob sums
  get_items_hi_probs(dr)
  items_hi_probs_list = []
  prob_sum = 0
  for el in dr.items_hi_probs.items() :
    tmp_list = [el[0], el[1]]
    prob_sum += el[1]
    tmp_list.append(prob_sum)
    items_hi_probs_list.append(tmp_list)
  
  print "users: ", len(dr.users)
  print "users_rs: ", len(dr.users_rs)
  
  # prepare job_server
  job_server = pp.Server()
  print "starting pp with", job_server.get_ncpus(), "workers"
      
  # go through all users and get validset (paralleling)
  print "submitting jobs"
  validsetnumb = 0
  users_jobs = []
  # submit jobs
  j = 0
  for user in dr.users.keys() :
    job = job_server.submit(get_user_valid_rs, (dr, user, items_hi_probs_list,), (get_rand_user_non_r, has_rated, get_rand_user_hi_r, has_3_hi_ratings), ('random', 'array') )
    users_jobs.append(job)
  
  print "doing jobs"
  # do jobs
  for job in users_jobs :
    #user, user_rs = job()
    user, user_rs = job()
    print user_rs
    #drawer.validset[user] = user_rs
    print "u=",user
    
    if (user_rs) :
      validsetnumb += 6
    if (validsetnumb > dr.validset_limit_r) :
      print validsetnumb
      return
      
def save_to_file(dr) :
  print 'Saving validset to text file... ', 
  file = open(dr.validset_fn + '.txt', 'w')
  
  for user in dr.validset :
    file.write('{0}|{1}\n'.format(user, len(dr.validset[user])) )
    for item, r in dr.validset[user].items() :
      file.write('{0}\t{1}\n'.format(item, r))
      
  print 'OK'
  file.close()  
    
""" MAIN """
def main() :
  dr = Drawer()
  draw_validset(dr)
  save_to_file(dr)

main()    
    
    
 