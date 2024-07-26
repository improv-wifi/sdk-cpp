# Example Programs

Build and run the examples with the following command:

```bash
# g++ -std=c++11 ../src/improv.cpp <example_file> && ./a.out

g++ -std=c++11 ../src/improv.cpp time_execution.cpp && ./a.out
g++ -std=c++11 ../src/improv.cpp show_rpc_response.cpp && ./a.out
```

## Show RPC Response

Show the constructed frame for a given command and vector string input. Prints the hex values as well as the frame and data length.

## Time Execution

Measure the time it takes to run 100.000 iterations of the library's functions. Prints it for the total and per call time.