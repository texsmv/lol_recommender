
import numpy as np
import pandas as pd
import csv


def charge_data(filename):
    l = list()
    with open(filename) as csv_file:
        csv_reader = list(csv.reader(csv_file, delimiter=','))
    return csv_reader


def read_file(filename):   
    equipos = charge_data(filename)
    l1 = []
    lista = []
    part, result = equipos[0][0],equipos[0][3]
    l1.append(part)
    for i in equipos:
        if(part != i[0]):
            r = 0 if result=='1' else 1
            l1.append(r)
            t = tuple(l1)
            lista.append(t)
            l1 = []
            part, result = i[0],i[3]
            l1.append(part)
            l1.append(i[1])
        else:
            l1.append(i[1])
    r = 0 if result=='1' else 1
    l1.append(r)
    t = tuple(l1)
    lista.append(t)
    dataframe_ids = pd.DataFrame(lista)
    dataframe_ids.to_csv("grupos.csv", index = False, header=False)
read_file("HeroPart.csv")

# charge_data("champions.csv")



