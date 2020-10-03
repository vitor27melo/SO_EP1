import plotly.express as px
import plotly.graph_objs as go
import numpy as np


def context_chgs_graph(FIFO, SRTN, RR, no_size):
    array_fifo = []
    array_srtn = []
    array_rr = []
 
    for i in range(1, 30):
        array_fifo.append(int(FIFO[i][len(FIFO[i])-1][0]))
        array_srtn.append(int(SRTN[i][len(SRTN[i])-1][0]))
        array_rr.append(int(RR[i][len(RR[i])-1][0]))

    media_fifo = np.mean(array_fifo)
    media_srtn = np.mean(array_srtn)
    media_rr = np.mean(array_rr)
    
    std_dev_fifo = np.std(array_fifo)
    std_dev_srtn = np.std(array_srtn)
    std_dev_rr = np.std(array_rr)

    intervalo_fifo = 0.475 * std_dev_fifo/np.sqrt(30)
    intervalo_srtn = 0.475 * std_dev_srtn/np.sqrt(30)
    intervalo_rr = 0.475 * std_dev_rr/np.sqrt(30)

    data = []
    data.append(media_fifo)
    data.append(media_srtn)
    data.append(media_rr)

    trace = go.Bar(x = ["FIFO","STRN","RR"],
                   y = data,
                   text = ["IC: [ " + str(np.around(media_fifo-intervalo_fifo,2)) + " , " + str(np.around(media_fifo+intervalo_fifo,2)) + " ]",
                   "IC: [ " + str(np.around(media_srtn-intervalo_srtn,2)) + " , " + str(np.around(media_srtn+intervalo_srtn,2)) + " ]", 
                   "IC: [ " + str(np.around(media_rr-intervalo_rr,2)) + " , " + str(np.around(media_rr+intervalo_rr,2)) + " ]"],
                   textposition='auto')

    data = [trace]

    layout = {"title": "Média do número de mudanças de contexto em um número de programas " + no_size + "s.",
            "xaxis":{"title":"Escalonador"},
            "yaxis":{"title":"Mudanças de contexto"}}

    fig = go.Figure(data = data,
                    layout = layout)

    fig.write_image("./graficos/context_chgs_"+no_size+".png")
    return 1

def deadline_graph(FIFO, SRTN, RR, no_size, input_file):

    array_fifo = [0] * 30
    array_srtn = [0] * 30
    array_rr = [0] * 30

    for i in range (30):
        for row in FIFO[i]:
            for input in input_file:
                if (input[0] == row[0]):
                    #decidir depois se dou um pequeno increase no input
                    if (float(row[2]) <= float(input[3])):
                        array_fifo[i] = array_fifo[i] + 1
                        break

        for row in SRTN[i]:
            for input in input_file:
                if (input[0] == row[0]):
                    #decidir depois se dou um pequeno increase no input
                    if (float(row[2]) <= float(input[3])):
                        array_srtn[i] = array_srtn[i] + 1
                        break

        for row in RR[i]:
            for input in input_file:
                if (input[0] == row[0]):
                    #decidir depois se dou um pequeno increase no input
                    if (float(row[2]) <= float(input[3])):
                        array_rr[i] = array_rr[i] + 1
                        break

    media_fifo = np.mean(array_fifo)
    media_srtn = np.mean(array_srtn)
    media_rr = np.mean(array_rr)
                    
    std_dev_fifo = np.std(array_fifo)
    std_dev_srtn = np.std(array_srtn)
    std_dev_rr = np.std(array_rr)

    intervalo_fifo = 0.475 * std_dev_fifo/np.sqrt(30)
    intervalo_srtn = 0.475 * std_dev_srtn/np.sqrt(30)
    intervalo_rr = 0.475 * std_dev_rr/np.sqrt(30)      

    data = []
    data.append(media_fifo)
    data.append(media_srtn)
    data.append(media_rr)        

    trace = go.Bar(x = ["FIFO","STRN","RR"],
                   y = data,
                   text = ["IC: [ " + str(np.around(media_fifo-intervalo_fifo,2)) + " , " + str(np.around(media_fifo+intervalo_fifo,2)) + " ]",
                   "IC: [ " + str(np.around(media_srtn-intervalo_srtn,2)) + " , " + str(np.around(media_srtn+intervalo_srtn,2)) + " ]", 
                   "IC: [ " + str(np.around(media_rr-intervalo_rr,2)) + " , " + str(np.around(media_rr+intervalo_rr,2)) + " ]"],
                   textposition='auto')

    data = [trace]

    layout = {"title": "Média do número de sucessos em atingir a deadline - Qtd. " + no_size + "(a) de programas.",
            "xaxis":{"title":"Escalonador"},
            "yaxis":{"title":"Sucessos deadline"}}

    fig = go.Figure(data = data, layout = layout)
    fig.update_yaxes(range=[0, len(input_file)])
    
    fig.write_image("./graficos/deadline_"+no_size+".png")

    return 1

def main():
    data_FIFO_pequeno  = []
    data_FIFO_medio  = []
    data_FIFO_grande  = []
    data_SRTN_pequeno  = []
    data_SRTN_medio  = []
    data_SRTN_grande  = []
    data_RR_pequeno  = []
    data_RR_medio  = []
    data_RR_grande  = []

    for i in range (30):
        with open("./testes/FIFO/trace_pequeno/resultado" + str(i)) as f:
            data_FIFO_pequeno.append([[cell.strip() for cell in row.rstrip(' ').split(' ')] for row in f]) 
        with open("./testes/FIFO/trace_medio/resultado" + str(i)) as f:
            data_FIFO_medio.append([[cell.strip() for cell in row.rstrip(' ').split(' ')] for row in f])
        with open("./testes/FIFO/trace_grande/resultado" + str(i)) as f:
            data_FIFO_grande.append([[cell.strip() for cell in row.rstrip(' ').split(' ')] for row in f])
            
        with open("./testes/SRTN/trace_pequeno/resultado" + str(i)) as f:
            data_SRTN_pequeno.append([[cell.strip() for cell in row.rstrip(' ').split(' ')] for row in f])
        with open("./testes/SRTN/trace_medio/resultado" + str(i)) as f:
            data_SRTN_medio.append([[cell.strip() for cell in row.rstrip(' ').split(' ')] for row in f])
        with open("./testes/SRTN/trace_grande/resultado" + str(i)) as f:
            data_SRTN_grande.append([[cell.strip() for cell in row.rstrip(' ').split(' ')] for row in f])
            
        with open("./testes/RR/trace_pequeno/resultado" + str(i)) as f:
            data_RR_pequeno.append([[cell.strip() for cell in row.rstrip(' ').split(' ')] for row in f])
        with open("./testes/RR/trace_medio/resultado" + str(i)) as f:
            data_RR_medio.append([[cell.strip() for cell in row.rstrip(' ').split(' ')] for row in f])
        with open("./testes/RR/trace_grande/resultado" + str(i)) as f:
            data_RR_grande.append([[cell.strip() for cell in row.rstrip(' ').split(' ')] for row in f])    
    
    with open("ep1_input_pequeno") as f:
            input_pequeno = [[cell.strip() for cell in row.rstrip(' ').split(' ')] for row in f]
    with open("ep1_input_medio") as f:
            input_medio = [[cell.strip() for cell in row.rstrip(' ').split(' ')] for row in f]
    with open("ep1_input_grande") as f:
            input_grande = [[cell.strip() for cell in row.rstrip(' ').split(' ')] for row in f]

    context_chgs_graph(data_FIFO_pequeno, data_SRTN_pequeno, data_RR_pequeno, "pequeno")
    context_chgs_graph(data_FIFO_medio, data_SRTN_medio, data_RR_medio, "medio")
    context_chgs_graph(data_FIFO_grande, data_SRTN_grande, data_RR_grande, "grande")

    deadline_graph(data_FIFO_pequeno, data_SRTN_pequeno, data_RR_pequeno, "pequeno", input_pequeno)
    deadline_graph(data_FIFO_medio, data_SRTN_medio, data_RR_medio, "medio", input_medio)
    deadline_graph(data_FIFO_grande, data_SRTN_grande, data_RR_grande, "grande", input_grande)
            
main()