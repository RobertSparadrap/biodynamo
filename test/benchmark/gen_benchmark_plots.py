#!/usr/bin/python3

import matplotlib.pyplot as plt
import numpy as np
import sys
from math import *
import json


def data_cpu(name_demo, iteration):
    cpu = [0]*iteration
    i = 0
    j = 0
    a = name_demo
    nb = len(name_demo)
    while i != iteration:
        b = data["benchmarks"][i]["name"]
        if a[:nb] == b[:nb]:
            cpu[j] = data["benchmarks"][i]["cpu_time"]
            j+=1
        i+=1
    return cpu

def graph(name_demo, iteration):
    cpu = data_cpu(name_demo, iteration)
    moy = 0
    h = 0
    tmp = 0
    while h < iteration:
        tmp = cpu[h] + tmp
        h += 1
    moy = tmp / iteration
    fig, ax = plt.subplots()
    xlabels = ['str']
    ax.plot([1], moy, 'bo-')
    ax.set_title(name_demo)
    ax.set_xticks([1])
    ax.set_xticklabels(xlabels, rotation=40)
    plt.show()
    return

def plot(name_demo, iteration):
    with open("results.json", "r") as read_file:
        data = json.load(read_file)
    graph(name_demo, iteration)

def name(i):
    name_data = data["benchmarks"][i]["name"]
    z = 0
    while z in range( len(name_data)):
        if name_data[z] == '/': break
        z += 1
    name_data = name_data[:z]
    return name_data

def names():
    name_datas = [0] * len(data["benchmarks"])
    i = 0
    j = 0
    while i != len(data["benchmarks"]):
        if name(i) != name_datas[j-1]:
            name_datas[j] = name(i)
            j+=1
        i+=1
    name_datas = name_datas[:j]
    return name_datas, j

def iteration():
    tmp = [0]
    i = 0
    x = 0
    j = 0
    while i != len(data["benchmarks"]):
        tmp = name(i)
        if tmp == name(i+1):
            j+=1
            i+=1
        else:
            break
    return j+1

def main():
    i = 0
    it = iteration()
    name_datas, j = names()
    while i < j:
        plot(name_datas[i], it)
        i += 1
    return

if __name__ == "__main__":
    file = sys.argv[1]
    with open(file, "r") as read_file:
        data = json.load(read_file)
    try:
        main()
    except:
        print("ERROR")