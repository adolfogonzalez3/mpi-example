#run:
#	mpirun -np 2 ./gradient_descent |& grep -v "Read -1"

build: gradient_descent.out homework3.out cannon.out oddevensorting.out
	echo "DONE"

gradient_descent.out: gradient_descent.cpp
	mpiCC -I /work/eigen/ -I /work/mnist/include/ gradient_descent.cpp -o gradient_descent.out

homework3.out: hmmm.cpp
	mpiCC hmmm.cpp -o homework3.out

oddevensorting.out: oddevensorting.cpp
	mpiCC oddevensorting.cpp -o oddevensorting.out

cannon.out: cannon.cpp
	mpiCC cannon.cpp -o cannon.out

clean:
	rm core.*