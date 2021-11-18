import numpy as np
np.random.seed(19680801)
import matplotlib
import matplotlib.pyplot as plt
import pandas as pd
import itertools
import os
import tkinter

matplotlib.use('TkAgg')



col_names = ['cycles', 'sram_read'] #reg_write
arr_h_list = [2,4,8,16,32]
arr_hw_list = [64]
sram_sz_list = [108]
data_flow_list = ['ws','is','os']

conv_hw_list = [16]
conv_fhw_list = [4]
conv_c_list = [4]
conv_filter_list = [16]#num_filter
conv_stride_list = [1]
configs = [arr_h_list, arr_hw_list, sram_sz_list, data_flow_list, conv_hw_list, conv_fhw_list, conv_c_list, conv_filter_list, conv_stride_list]
exp_settings = list(itertools.product(*configs))
exp_settings = list(list(es) for es in exp_settings)
for es in exp_settings:
  es[1] = int(es[1]/es[0])
  
csv_name = os.path.join("/home/qino/Desktop/event_queue/eval/optimal-dataflow-arraysize", "summary2.csv")
data = pd.read_csv(csv_name)
for dataflow in data_flow_list:
  fig, ax = plt.subplots()

  xticks = []
  y = []
  color = 'tab:red'
  for es in exp_settings:
    offset = "_".join([str(e) for e in es])
    print(offset, data[data['identifier'].str.contains(offset)]['cycles'])
    if data[data['identifier'].str.contains(offset)]['cycles'].empty:
      continue
    if es[3]!=dataflow: continue
    y.append(data[data['identifier'].str.contains(offset)]['cycles'].item() )
    xticks = xticks + [str(es[0])+"x"+str(es[1])]
  x = range(len(xticks))
  print(x, y)
  ax.plot(x, y, color=color, alpha=0.8, markersize=10, marker='X')
  ax.set_ylabel('Cycles',color=color)
  ax.tick_params(axis='y', labelcolor=color)
  ax.set_xticks(x)
  ax.set_xticklabels(xticks)

  ax2 = ax.twinx()
  y = []
  color = 'tab:blue'
  for es in exp_settings:
    if es[3]!=dataflow: continue
    offset = "_".join([str(e) for e in es])
    if data[data['identifier'].str.contains(offset)]['sram_read'].empty:
      continue
    y.append(data[data['identifier'].str.contains(offset)]['sram_read'].item() )
  print(x, y)
  ax2.plot(x, y, color=color, alpha=0.8, markersize=10, marker='o')
  ax2.set_ylabel('Bandwidth', color=color)
  ax2.tick_params(axis='y', labelcolor=color)
  #ax.legend()
  ax.grid(True)
  #plt.title('My title')
  plt.xlabel('Array Size')
  #plt.ylabel(col_n)

  plt.savefig('oneset_'+dataflow+'.png')
  
  
  
