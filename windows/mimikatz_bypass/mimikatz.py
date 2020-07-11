import base64
import random
import string

mimiakatz_file=open('mimikatz.exe','rb')
base64_str = base64.b64encode(mimiakatz_file.read())
print base64_str
