# Optimization Project Directory Structure

This project has been organized into the following descriptive directory structure:

## Directories

- **module_main_optimizer/**: Source code and outputs for the main optimization algorithm (`opti.cc`)
  - `opti.cc` - Main optimization algorithm source code
  - `output.txt` - Output from the main optimization algorithm

- **executables_bin/**: Compiled executable files for all modules
  - `a.exe` - Generic executable (if used)
  - `calculate_opti_value.exe` - Executable for the OptiValue calculator
  - `opti.exe` - Executable for the main optimization algorithm

- **project_general_inputs/**: General input files used by various modules
  - `input_0.txt`, `input_1.txt`, `input_2.txt` - General test input files
  - `input.txt` - Main general input file (if used by multiple modules)
  - `question.txt` - Project requirements or problem description

- **module_opti_value_calculator/**: Source code, specific inputs, and analysis for the OptiValue calculator (`calculate_opti_value.cc`)
  - `calculate_opti_value.cc` - OptiValue calculator source code
  - `graph_input.txt` - Graph structure input for OptiValue calculator
  - `paths_input.txt` - Path definitions for OptiValue calculator
  - `analysis_output.txt` - Detailed analysis results from OptiValue calculator
  - `REPORT_on_opti_value.md` - Report analyzing OptiValue calculation

- **docs/**: General project documentation (To be created if not present)
  - `README.md` - Overall project documentation
  - `DIRECTORY_GUIDE.md` - This file, explaining the directory structure

## Usage & I/O Paths

Updated commands and file references:

- **Main Optimizer (`opti.cc`):**
  - Reads from `project_general_inputs/input_2.txt` by default; edit `freopen` to change.
  - Writes to `module_main_optimizer/output.txt` by default.
  ```cpp
  freopen("project_general_inputs/input_2.txt","r",stdin);
  freopen("module_main_optimizer/output.txt","w",stdout);
  ```

- **OptiValue Calculator (`calculate_opti_value.cc`):**
  - Redirects `stdout` to `module_opti_value_calculator/analysis_output.txt`.
  - Loads graph from `module_opti_value_calculator/graph_input.txt` and paths from `module_opti_value_calculator/paths_input.txt`.
  ```cpp
  freopen("module_opti_value_calculator/analysis_output.txt","w",stdout);
  readGraph("module_opti_value_calculator/graph_input.txt");
  std::ifstream paths_file("module_opti_value_calculator/paths_input.txt");
  ```

*Ensure these paths match your project structure when building or moving files.*

## Building Code

```powershell
# To build the OptiValue calculator:
cd "c:\Users\bilas\Documents\VS CODE\opti"
g++ -g .\module_opti_value_calculator\calculate_opti_value.cc -o .\executables_bin\calculate_opti_value.exe

# To build the main optimizer:
g++ -g .\module_main_optimizer\opti.cc -o .\executables_bin\opti.exe
```

### Running Executables

Run executables from the root project directory (`c:\Users\bilas\Documents\VS CODE\opti`):

```powershell
# Run OptiValue calculator:
.\executables_bin\calculate_opti_value.exe

# Run main optimization algorithm:
.\executables_bin\opti.exe
```

### File Paths in Code

When modifying code, ensure file paths correctly reference the new directory structure. For example, if `calculate_opti_value.exe` (running from root) needs `graph_input.txt`:

```cpp
// In module_opti_value_calculator/calculate_opti_value.cc
string graph_file = "module_opti_value_calculator/graph_input.txt";
string paths_file = "module_opti_value_calculator/paths_input.txt";
freopen("module_opti_value_calculator/analysis_output.txt", "w", stdout);
```

And if `opti.exe` (running from root) needs `input.txt` from general inputs and writes to its module's `output.txt`:

```cpp
// In module_main_optimizer/opti.cc
freopen("project_general_inputs/input.txt", "r", stdin);
freopen("module_main_optimizer/output.txt", "w", stdout);
```

**Note:** This structure helps in organizing files based on their module and type, improving project clarity and maintainability.
