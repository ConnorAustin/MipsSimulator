import sys, re

def to_digit(num):
    try:
        return (int(num), False)
    except ValueError:
        try:
            return (int(num, 16), True) if len(num) > 2 and num[:2] == "0x" else None
        except ValueError:
            return None

base_address = 0 if len(sys.argv) < 3 else to_digit(sys.argv[2])[0] if to_digit(sys.argv[2]) != None and to_digit(sys.argv[2])[0] % 4 == 0 else err("'" + sys.argv[2] + "' is not a valid base address")
if base_address > 2**32 - 1:
    err("The base address is not in the 32-bit address space and you know it sister")
my_vars = {"label_table": {}, "fout": open("a.out", "w"), "fin": None, "lst2": [], "addr": base_address, "registers": ["$zero", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3", "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7", "$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra"] }

try:
    my_vars["fin"] = open(sys.argv[1], "r") if len(sys.argv) > 1 else err("Requires input file")
except IOError:
    err("Could not open file '" + sys.argv[1] + "'")
    
def err(msg):
    print("An error has occurred:\n" + msg + "\nExiting")
    a = (my_vars["fout"].seek(0), my_vars["fout"].truncate(), my_vars["fout"].close(), sys.exit(0))
    
def find(lst, lam):
    return lst.index(list(filter(lam, lst))[0]) if len(list(filter(lam, lst))) != 0 else None
    
def get_register(reg):
    return find(my_vars["registers"], lambda r: r == reg) if find(my_vars["registers"], lambda r: r == reg) != None else err("Unknown register: '" + reg + "'")
    
def assert_instruction_len(words, size):
    return err("Excessive instruction token: '" + words[size] + "'") if len(words) > size else err("Missing an instruction token") if len(words) < size else None
            
def parse_r(ins, words, address):
    assert_instruction_len(words, 4)
    my_vars["fout"].write("0x{0:08X}:    ".format(address) + "{0:032b}    0x{0:08X}\n".format((get_register(words[2]) << 21) | (get_register(words[3]) << 16) | get_register(words[1]) << 11 | ins[2]))

def parse_shift(ins, words, address):
    assert_instruction_len(words, 4)
    if to_digit(words[3]) == None:
        err("'" + words[3] + "' is not a valid argument for " + words[0])
    if to_digit(words[3])[0] > 2**5 - 1 or to_digit(words[3])[0] < 0:
        err("'" + words[3] + "' is not a valid number for " + words[0])
    my_vars["fout"].write("0x{0:08X}:    ".format(address) + "{0:032b}    0x{0:08X}\n".format((get_register(words[2]) << 16) | (get_register(words[1]) << 11) | (to_digit(words[3])[0] << 6) | ins[2]))
    
def parse_jr(ins, words, address):
    assert_instruction_len(words, 2)
    my_vars["fout"].write("0x{0:08X}:    ".format(address) + "{0:032b}    0x{0:08X}\n".format((ins[1] << 26) | (get_register(words[1]) << 21) | ins[2]))

def parse_i(ins, words, address):
    assert_instruction_len(words, 4)
    if to_digit(words[3]) == None:
        err("'" + words[3] + "' is not a valid argument for " + words[0])
    if to_digit(words[3])[1] and to_digit(words[3])[0] > 2**16 - 1 or (not to_digit(words[3])[1] and (to_digit(words[3])[0] < -(2**15) or to_digit(words[3])[0] > 2**15 - 1)):
        err("'" + words[3] + "' is too large in magnitude of a number for " + words[0])
    my_vars["fout"].write("0x{0:08X}:    ".format(address) + "{0:032b}    0x{0:08X}\n".format((ins[1] << 26) | (get_register(words[1]) << 16) | (get_register(words[2]) << 21) | (to_digit(words[3])[0] & 0xFFFF)))

def parse_branch(ins, words, address):
    assert_instruction_len(words, 4)
    if not words[3] in my_vars["label_table"]:
        err("'" + words[3] + "' label does not exist")
    my_vars["fout"].write("0x{0:08X}:    ".format(address) + "{0:032b}    0x{0:08X}\n".format((ins[1] << 26) | (get_register(words[1]) << 21) | (get_register(words[2]) << 16) | (((my_vars["label_table"][words[3]] - (address + 4)) >> 2) & 0xFFFF)))

def parse_lwsw(ins, words, address):
    assert_instruction_len(words, 4)
    if words[3][0] != "(" or words[3][len(words[3]) - 1] != ")":
        err("Missing parentheses for " + words[0])
    if to_digit(words[2]) == None:
        err("'" + words[2] + "' is not a valid argument for " + words[0])
    if to_digit(words[2])[1] and to_digit(words[2])[0] > 2**16 - 1 or (not to_digit(words[2])[1] and (to_digit(words[2])[0] < -(2**15) or to_digit(words[2])[0] > 2**15 - 1)):
        err("'" + words[2] + "' is too large in magnitude of a number for " + words[0])
    my_vars["fout"].write("0x{0:08X}:    ".format(address) + "{0:032b}    0x{0:08X}\n".format((ins[1] << 26) | (get_register(words[3][1:][:-1]) << 21) | (get_register(words[1]) << 16) | (to_digit(words[2])[0] & 0xFFFF)))

def parse_j(ins, words, address):
    assert_instruction_len(words, 2)
    if not words[1] in my_vars["label_table"]:
        err("'" + words[1] + "' label does not exist")
    my_vars["fout"].write("0x{0:08X}:    ".format(address) + "{0:032b}    0x{0:08X}\n".format((ins[1] << 26) | ((my_vars["label_table"][words[1]] >> 2) & 0xFFFFFF)))
    
instructions = [("add", 0, 0x20, parse_r), ("sub", 0, 0x22, parse_r), ("and", 0, 0x24, parse_r), ("or", 0, 0x25, parse_r), ("nor", 0, 0x27, parse_r), ("slt", 0, 0x2A, parse_r), ("sll", 0, 0x0, parse_shift), ("srl", 0, 0x2, parse_shift), ("jr", 0, 0x8, parse_jr), ("addi", 0x8, -1, parse_i), ("andi", 0xC, -1, parse_i), ("ori", 0xD, -1, parse_i), ("beq", 0x4, -1, parse_branch), ("bne", 0x5, -1, parse_branch), ("lw", 0x23, -1, parse_lwsw), ("sw", 0x2B, -1, parse_lwsw), ("j", 0x2, -1, parse_j), ("jal", 0x3, -1, parse_j)]

lst = filter(lambda l: len(l.strip()) != 0, re.sub(r"#.*\n|$", "\n", my_vars["fin"].read().replace(":", ":\n")).split("\n"))
for line in lst:
    if ":" in line:
        if not line.replace(":", "").isalnum() and line.replace(":", "").isdigit():
            err("'" + line + "' is not a valid label name")
        if line in my_vars["label_table"]:
            err("The label '" + line + "' is already defined")
        my_vars["label_table"][line[:-1]] = my_vars["addr"]
    else:
        my_vars["addr"] += 4
        if my_vars["addr"] > 2**32 - 1:
            err("Ran out of 32-bit address space")
        my_vars["lst2"].append(re.sub(r"\s*\)", ")", re.sub(r"\(\s*", " (", line.replace(",", ""))).split())
for i in range(len(my_vars["lst2"])):
    ins = find(instructions, lambda n: n[0] == my_vars["lst2"][i][0])
    a = instructions[ins][3](instructions[ins], my_vars["lst2"][i], base_address + i * 4) if ins != None else err("Unknown instruction: '" + my_vars["lst2"][i][0] + "'")