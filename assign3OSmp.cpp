#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstdlib>
#include <ctime>
#include <limits.h>

using namespace std;

#define SIZE 20

// Function to find the minimum value in an array segment
int findMin(int arr[], int start, int end) {
    int minVal = INT_MAX;
    for (int i = start; i < end; i++) {
        if (arr[i] < minVal) {
            minVal = arr[i];
        }
    }
    return minVal;
}

int main() {
    int arr[SIZE];
    srand(time(0));

    // Fill array w/ random numbers
    cout << "Generated array: ";
    for (int i = 0; i < SIZE; i++) {
        arr[i] = rand() % 100;
        cout << arr[i] << " ";
    }
    cout << endl;

    int fd[2]; // Pipe for interprocess communication
    if (pipe(fd) == -1) {
        cerr << "Error: Pipe creation failed!" << endl;
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        cerr << "Error: Fork failed!" << endl;
        return 1;
    }

    if (pid == 0) { // Child Process
        close(fd[0]); // Close read end
        int minChild = findMin(arr, SIZE / 2, SIZE);

        if (write(fd[1], &minChild, sizeof(minChild)) == -1) {
            cerr << "Error: Write to pipe failed!" << endl;
            exit(1);
        }

        close(fd[1]); // Close write end
        cout << "Child Process ID: " << getpid() << " - Min in second half: " << minChild << endl;
        exit(0);
    } else { // Parent Process
        close(fd[1]); // Close write end
        int minParent = findMin(arr, 0, SIZE / 2);
        int minChild;

        if (read(fd[0], &minChild, sizeof(minChild)) == -1) {
            cerr << "Error: Read from pipe failed!" << endl;
            return 1;
        }

        close(fd[0]); // Close read end

        // Ensure the child process has finished
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            cout << "Parent Process ID: " << getpid() << " - Min in first half: " << minParent << endl;
            cout << "Overall Minimum: " << min(minParent, minChild) << endl;
        } else {
            cerr << "Error: Child process terminated abnormally." << endl;
            return 1;
        }
    }

    return 0;
}
