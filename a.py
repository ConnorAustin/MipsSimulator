out = open("save.new.txt", "w")
i = open("save.txt", "r")
first = 1
for line in i:
	if first == 1:
		first = 0
		out.write(line.strip() + "\n")
		continue
	x = int(line.split()[0])
	y = int(line.split()[1])
	x += 20
	out.write(str(x) + " " + str(y) + "\n")
i.close()
out.close()
