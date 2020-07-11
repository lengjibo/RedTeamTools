import fileinput
import base64



with open("./Win32/mimikatz.exe", "rb") as f:
    win32 = base64.b64encode(f.read()).decode()

with open("./x64/mimikatz.exe", "rb") as f:
    x64 = base64.b64encode(f.read()).decode()


for line in fileinput.FileInput("./Invoke-Mimikatz.ps1", inplace=1):

	line = line.rstrip('\r\n')
	if "$PEBytes64 = " in line:
		print("$PEBytes64 = '" + x64 + "'")
	elif "$PEBytes32 = " in line:
		print("$PEBytes32 = '" + win32 + "'")
	else:
		print(line)
