"""
This script generates documentation for command functions in the Nebulite project.
It extracts command information from both GlobalSpace and RenderObject domains
and outputs comprehensive documentation in markdown format.
"""

#####################################
# Imports
import subprocess
import re
import os
import sys
import shutil
from typing import List, Dict, Tuple, Optional

#####################################
# Command configurations

# Define the root commands for GlobalSpace and RenderObject
# We simply need to add "help" to these commands to get the list of available functions/variables
ROOT_GLOBALSPACE = "./bin/Nebulite <arg>"
ROOT_RENDEROBJECT = "./bin/Nebulite draft parse <arg>"
ROOT_JSON_TRANSFORMATIONS = "./bin/Nebulite eval nop {global.var|<arg>}"

# Output file for the generated documentation
OUTPUT_FILE = "./doc/Commands.md"
TIMEOUT_SECONDS = 3  # Timeout for commands that might open renderer windows

#####################################
# Functions

# Markdown formatting helpers
class MarkdownFormatter:
    def get_header(self, name: str, level: int) -> str:
        if name == 'root' or not name:
            return ""
        header_prefix = "#" * min(level, 6)
        return f"{header_prefix} `{name}`\n\n"

    def get_intro(self, help_text: str) -> str:
        if not help_text:
            return ""
        for line in help_text.split('\n'):
            if 'Help for' in line:
                return f"{line.strip()}\n\n"
        return ""

    def get_description(self, help_text: str) -> str:
        if not help_text:
            return ""
        lines = help_text.split('\n')
        description_started = False
        description_lines = []
        for line in lines:
            if description_started:
                if not line.startswith('Available') and not line.startswith('Add the entries'):
                    description_lines.append(line)
                elif line.startswith('Available') or line.startswith('Add the entries'):
                    break
            elif 'Help for function' in line or 'Help for' in line:
                description_started = True
        # Remove empty lines at the beginning and end
        while description_lines and not description_lines[0].strip():
            description_lines.pop(0)
        while description_lines and not description_lines[-1].strip():
            description_lines.pop()
        if description_lines:
            return "```\n" + '\n'.join(description_lines) + "\n```\n\n"
        return ""

    def get_functions_table(self, functions, help_text):
        if not functions:
            return ""
        table = "Available Functions\n\n| Function | Description |\n|----------|-------------|\n"
        help_lines = help_text.split('\n') if help_text else []
        func_descriptions = {}
        for line in help_lines:
            match = re.match(r'\s+([a-zA-Z0-9_-]+)\s+-\s+(.+)', line)
            if match:
                func_descriptions[match.group(1)] = match.group(2)
        for func in functions:
            desc = func_descriptions.get(func, "No description available")
            table += f"| `{func}` | {desc} |\n"
        return table + "\n"

    def get_variables_table(self, variables, help_text):
        if not variables:
            return ""
        table = "Available Variables\n\n| Variable | Description |\n|----------|-------------|\n"
        help_lines = help_text.split('\n') if help_text else []
        var_descriptions = {}
        in_variables_section = False
        for line in help_lines:
            if line.startswith('Available variables:'):
                in_variables_section = True
                continue
            if in_variables_section and line.strip():
                match = re.match(r'\s+([a-zA-Z0-9_-]+)\s+-\s+(.+)', line)
                if match:
                    var_descriptions[match.group(1)] = match.group(2)
        for var in variables:
            desc = var_descriptions.get(var, "No description available")
            table += f"| `--{var}` | {desc} |\n"
        return table + "\n"

    def get_subcommands(self, subcommands, level, format_section_func):
        if not subcommands:
            return ""
        return ''.join(format_section_func(subcmd_data, level + 1) for subcmd_data in subcommands.values())

def run_command_with_timeout(command: str, timeout: int = TIMEOUT_SECONDS) -> Optional[str]:
    """
    Execute a command with timeout to handle cases where renderer windows are opened.
    Returns the command output or None if timeout/error occurs.
    """
    try:
        # Split command into list to avoid shell=True security issue
        import shlex
        cmd_parts = shlex.split(command)
        
        # Separate environment variables from the actual command
        env_vars = {}
        cmd_list = []
        
        for part in cmd_parts:
            if '=' in part and not cmd_list:  # Environment variables come before the command
                key, value = part.split('=', 1)
                env_vars[key] = value
            else:
                cmd_list.append(part)
        
        # Merge with current environment
        import os
        env = os.environ.copy()
        env.update(env_vars)
        
        result = subprocess.run(cmd_list, env=env, capture_output=True, text=True, timeout=timeout)
        return result.stdout if result.returncode == 0 else None
    except subprocess.TimeoutExpired:
        print(f"Warning: Command '{command}' timed out after {timeout} seconds")
        return None
    except Exception as e:
        print(f"Error executing command '{command}': {e}")
        return None


def parse_command_list(help_output: str) -> Tuple[List[str], List[str]]:
    """
    Parse help output to extract function names and variable names.
    Returns tuple of (functions, variables).
    """
    functions = []
    variables = []
    
    if not help_output:
        return functions, variables
    
    lines = help_output.split('\n')
    current_section = None
    
    for line in lines:
        stripped_line = line.strip()
        if stripped_line.startswith("Available functions:"):
            current_section = "functions"
            continue
        elif stripped_line.startswith("Available variables:"):
            current_section = "variables"
            continue
        
        # Parse function/variable entries
        if current_section and line and not stripped_line.startswith("Add the entries"):
            # Match pattern like "  add-clock                 - Description"
            # Note: Using original line (with whitespace) for the regex match
            match = re.match(r'\s+([a-zA-Z0-9_-]+)\s+-\s+(.+)', line)
            if match:
                name = match.group(1)
                if current_section == "functions":
                    functions.append(name)
                elif current_section == "variables":
                    variables.append(name)
    
    return functions, variables


def get_command_help(base_command: str, command_name: str = "") -> Optional[str]:
    """
    Get detailed help for a specific command.
    """
    if command_name:
        full_command = f"{base_command} help {command_name}"
    else:
        full_command = f"{base_command} help"
    
    # Add exit command to ensure clean termination for all commands
    full_command += " '; exit'"
    
    return run_command_with_timeout(full_command)


def has_subcommands(help_output: str) -> bool:
    """
    Check if a command has subcommands by looking for "Add the entries name" pattern.
    """
    return bool(help_output and "Add the entries name to the command for more details:" in help_output)


def process_command_recursively(base_command: str, command_name: str = "",
                                prefix: str = "", visited: Optional[set] = None) -> Dict:
    """
    Recursively process a command and its subcommands.
    Supports replacing the template token `<arg>` in `base_command` with:
      - 'help' for listing functions
      - 'help <function>' when querying a specific function

    If `<arg>` is present the original `base_command` is preserved for recursion
    so nested substitutions continue to work.
    """
    if visited is None:
        visited = set()

    # Avoid infinite recursion
    full_name = f"{prefix}{command_name}".strip()
    if full_name in visited:
        return {}
    visited.add(full_name)

    result = {
        'name': command_name or 'root',
        'full_name': full_name,
        'help': None,
        'functions': [],
        'variables': [],
        'subcommands': {}
    }

    # Build and run help command, substituting '<arg>' if present
    help_output = None
    try:
        insert = "help" if not command_name else f"help {command_name}"
        if "<arg>" in base_command:
            help_cmd = base_command.replace("<arg>", insert)
            # Ensure termination for interactive commands
            if not help_cmd.strip().endswith("'; exit'"):
                help_cmd = help_cmd + " '; exit'"
            help_output = run_command_with_timeout(help_cmd)
        else:
            help_output = get_command_help(base_command, command_name)
    except Exception as e:
        print(f"Error building help command: {e}")
        help_output = None

    if not help_output:
        return result

    result['help'] = help_output

    # Parse functions and variables
    functions, variables = parse_command_list(help_output)
    result['functions'] = functions
    result['variables'] = variables

    # Check if this command has subcommands and process them
    if has_subcommands(help_output):
        for func in functions:
            # Skip 'help' function to avoid recursion
            if func == 'help':
                continue

            # For GlobalSpace, avoid exploring "draft parse" subcommands
            if not command_name and func == 'draft' and base_command == ROOT_GLOBALSPACE:
                continue

            # Preserve template base_command if it contains '<arg>', otherwise append the current command_name
            if "<arg>" in base_command:
                new_base = base_command
            else:
                new_base = f"{base_command} {command_name}".strip() if command_name else base_command

            new_prefix = f"{prefix}{command_name} " if command_name else prefix

            subcommand_result = process_command_recursively(
                new_base, func, new_prefix, visited
            )
            if subcommand_result:
                result['subcommands'][func] = subcommand_result

    return result


def format_markdown_section(command_data: Dict, level: int = 1) -> str:
    """
    Format command data into markdown section using MarkdownFormatter helpers.
    """
    if not command_data:
        return ""

    formatter = MarkdownFormatter()
    name = command_data.get('full_name', command_data.get('name', 'Unknown'))
    help_text = command_data.get('help', '')
    functions = command_data.get('functions', [])
    variables = command_data.get('variables', [])
    subcommands = command_data.get('subcommands', {})

    markdown = ""
    if name == 'root' or not name:
        markdown += formatter.get_intro(help_text)
    else:
        markdown += formatter.get_header(name, level)
        markdown += formatter.get_description(help_text)
    markdown += formatter.get_functions_table(functions, help_text)
    markdown += formatter.get_variables_table(variables, help_text)
    markdown += formatter.get_subcommands(subcommands, level, format_markdown_section)
    return markdown


def generate_documentation(date: str) -> str:
    """
    Generate complete documentation for both GlobalSpace and RenderObject domains.
    """


    # Begin markdown document
    markdown = "# Nebulite Command Documentation\n\n"
    markdown += "This documentation is automatically generated.\n\n"
    markdown += f"Generated on: {date}\n\n"

    # Insert a simple manual Table of Contents for the two main sections
    markdown += "## Table of Contents\n\n"
    markdown += "- [GlobalSpace Commands](#globalspace-commands)\n"
    markdown += "- [RenderObject Commands](#renderobject-commands)\n\n"
    markdown += "- [JSON Transformations Commands](#json-transformations-commands)\n\n"

    # Process GlobalSpace commands
    print("Processing GlobalSpace commands...")
    markdown += "## GlobalSpace Commands\n\n"
    markdown += "These commands are available in the global Nebulite namespace.\n\n"
    
    global_data = process_command_recursively(ROOT_GLOBALSPACE)
    if global_data and global_data.get('help'):
        print(f"Found {len(global_data.get('functions', []))} functions and {len(global_data.get('variables', []))} variables")
        markdown += format_markdown_section(global_data, level=3)
    else:
        markdown += "Failed to retrieve GlobalSpace commands.\n\n"
        print("No global data retrieved")
    
    # Process RenderObject commands
    print("Processing RenderObject commands...")
    markdown += "## RenderObject Commands\n\n"
    markdown += "These commands are available in the RenderObject domain (accessed via `draft parse`).\n\n"
    
    render_data = process_command_recursively(ROOT_RENDEROBJECT)
    if render_data and render_data.get('help'):
        print(f"Found {len(render_data.get('functions', []))} RenderObject functions and {len(render_data.get('variables', []))} variables")
        markdown += format_markdown_section(render_data, level=3)
    else:
        markdown += "Failed to retrieve RenderObject commands.\n\n"
        print("No render data retrieved")

    # Process JSON Transformations commands
    print("Processing JSON Transformations commands...")
    markdown += "## JSON Transformations Commands\n\n"
    markdown += "These commands are available during JSON value retrieval with the transformation operator '|'.\n"
    markdown += "Example: `{global.var|length}`\n\n"

    json_transform_data = process_command_recursively(ROOT_JSON_TRANSFORMATIONS)
    if json_transform_data and json_transform_data.get('help'):
        print(f"Found {len(json_transform_data.get('functions', []))} JSON transformation functions and {len(json_transform_data.get('variables', []))} variables")
        markdown += format_markdown_section(json_transform_data, level=3)
    else:
        markdown += "Failed to retrieve JSON transformation commands.\n\n"
        print("No JSON transformation data retrieved")
    
    return markdown


def main():
    """
    Main function to generate and save command documentation.
    """
    print("Starting Nebulite command documentation generation...")

    # See if command date exists
    if not shutil.which("date"):
        print("date command not found. Please install coreutils package.")
        print("On Ubuntu/Debian: sudo apt install coreutils")
        print("On Fedora: sudo dnf install coreutils")
        sys.exit(1)

    # Get full path to date
    date_path = shutil.which("date")
    date = subprocess.run([f'{date_path}'], capture_output=True, text=True).stdout.strip()
    
    # Check if Nebulite executable exists
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.join(script_dir, "..")
    nebulite_path = os.path.join(project_root, "bin", "Nebulite")
    
    if not os.path.exists(nebulite_path):
        print(f"Error: Nebulite executable not found at {nebulite_path}")
        print("Please build the project first using the build script.")
        sys.exit(1)
    
    try:
        # Generate documentation
        documentation = generate_documentation(date)
        
        # Write to output file
        output_path = os.path.join(project_root, OUTPUT_FILE)
        with open(output_path, 'w', encoding='utf-8') as f:
            f.write(documentation)
        
        print(f"Documentation successfully generated: {output_path}")
        print(f"Total length: {len(documentation)} characters")
        
    except KeyboardInterrupt:
        print("\nDocumentation generation interrupted by user.")
        sys.exit(130)
    except Exception as e:
        print(f"Error generating documentation: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
