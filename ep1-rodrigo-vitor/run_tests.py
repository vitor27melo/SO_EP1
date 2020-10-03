from subprocess import call

testes = [[1,"FIFO"],[2,"SRTN"],[3,"RR"]]

sizes = ["pequeno","medio","grande"]

for teste in testes:           
    for size in sizes:
        for i in range(30):
            print("Executando " + str(teste[1]) + " scheduler de tamanho " + size + ": " + str(i+1) + " de 30.")
            call(["./ep1",str(teste[0]),"ep1_input_" + size,"./testes/" + teste[1] + "/trace_" + size + "/resultado" + str(i)])
            