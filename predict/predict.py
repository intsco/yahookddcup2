# solution.py
# -*- coding: utf-8 -*-

import random
import sys
import math
import cPickle
import array
import time
sys.path.append('../common')
from loader import *
from estimator import *
from pred_taxonomy import *
from content_pred import *

print "-> Prediction started"
start = time.time()

train_fn = '../train_sample'
valid_fn = '../valid_sample'

# load data
train = load_trainset2(train_fn)
valid = load_validset(valid_fn)

# study and predict

#pred_tax = pred_taxonomy()
#pred_tax.study(users_rs)
#testset = pred_tax.predict(users, users_rs, testset)

#	content-based prediction
content_pred = Content_pred()
content_pred.study(train, valid)
content_pred.predict()

# true random prediction
#predict_random()
# simple taxonomy predictions
#predict_taxonomy(users, users_rs, testset_dict)

# estimate
estimate(valid_fn, valid)

print '-> finished ({0} sec)'.format(time.time()-start)









