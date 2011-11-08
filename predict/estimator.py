# -*- coding: utf-8 -*-

def estimate(fn, testset) :
  print "-> Estimation started"

  test_file = open(fn + '.txt', 'r')
  
  errn = 0
  n = 0
  
  for line in test_file.readlines() :
    if (line.find('|') >= 0) :
      elements = line.split('|')
      user = int(elements[0])
    else :
      elements = line.split('\t')
      item = int(elements[0])
      rating = int(elements[1])
      pred_rating = testset[user][item]
      if (rating < 1) : rating = 0
      else : rating = 1
      if (rating != pred_rating) : errn += 1
      n += 1
      #print "r = %d _r = %d" % (rating, pred_rating)
  
  print "-> Error rate: {0} %".format(float(errn) / n * 100)
  test_file.close()
  return
  