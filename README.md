# TechShell Project
## By Dawson Benison and Collin Songy
Each member seperately worked on thier own versions of the project, while also discussing the best way to complete the project. Once both partners were finished the more finished implementation was chosen.
### Overview
A C implementation of a simple command line shell. Executes commands, has built in funcitons cd and exit. Handles I/O redirection opereators, <, >, >>, 2>, 2>>.

# Usage
## 1. Clone the repository
```bash
git clone https://github.com/Z728x/TechShell
cd TechShell
```

## 2. Compilation
```bash 
gcc techshell.c -o techshell 
```

## 3. Run the shell 
```
./techshell 
```

## Example Commands
```bash
ls -l
cd /path/to/directory
echo "Hello World" > output.txt
wc < output.txt > wc.out
```

