# -*- coding: utf-8 -*-

import random, sys, math, array

""" Predictors """

def predict_random() :
  for user, items in users['test'].items() :
    for item, rating in items.items() :
      rval = random.randint(0, 1)
      users['test'][user][item] = rval
  return

class Pred_taxonomy() :
  items_numb = {} # item -> number of ratings
  
  def study(self, users_rs) :
    print '-> study: prapare item numbers'
    
    j = 0
    print "users_rs ", len(users_rs)
    n = len(users_rs)
    while (j < n) :
      item = users_rs[j]
      if (not self.items_numb.has_key(item)) :
        self.items_numb[item] = 1
      else :
        self.items_numb[item] += 1
      j += 2
    print "items_numb ", len(self.items_numb)

  def predict(self, users, users_rs, testset) :
    print "-> prediction: taxonomy, item ratings number"

    for user, items in testset.items() :
      user_items_numb = []
      for item in items.keys() :
        user_items_numb.append( (item, self.items_numb[item], 1))
      user_items_numb = sorted(user_items_numb, key=lambda item: item[1])
      
      for j in range(3) :
        user_items_numb[j] = (user_items_numb[j][0], user_items_numb[j][0], 0)
      
      for i, n, f in user_items_numb :
        testset[user][i] = f  
    return testset
  
  
  
  
  
  