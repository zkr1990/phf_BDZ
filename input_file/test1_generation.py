import sys
import os
import random

if __name__=='__main__':
    print('begin')
    max_num = 400
    total = 40
    #if (16**n_bit < total * 2):
    #    raise('number of bit is too short')
    result_key = []
    count = 0
    while(count < total):
	tmp_key = str(int(random.uniform(0,max_num)))
	if (tmp_key not in result_key):
		result_key.append(tmp_key)
                count += 1
    print('number of generated key is: ', len(result_key))
    print('writing to txt file...')
    f = open('input.txt','w')
    #f.write(str(max_num))
    #f.write('\n')
    #f.write(str(total))
    #f.write('\n')
    s = '\n'
    f.write(s.join(result_key))
    f.close()
    print('finish.')


