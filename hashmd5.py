#imports
import os, sys
import time
import csv
import hashlib
import subprocess

print("Hello, World!")

# configuration variables
# dir = r'C:\Users\Desktop\x2'
BUF_SIZE = 65536
dir = r"F:\dir"
csv_path = r"F:\filenames.csv"
csv_header = ['filename','md5',  'filesize', 'md5hex', 'date_modified']

# generate hash
def hashFunction(filename):
  with open(filename, "rb") as f:
    file_hash = hashlib.md5()
    while chunk := f.read(8192):
      file_hash.update(chunk)
      
    md5digest = file_hash.digest()
    md5hexdigest = file_hash.hexdigest()
    print(file_hash.digest())
    print(file_hash.hexdigest())
    
    return md5digest, md5hexdigest 

count = 0

csv_file = open(csv_path, 'w', encoding='UTF8', newline='')
writer = csv.writer(csv_file)
writer.writerow(csv_header)
 
for root, dirs, files in os.walk(dir):
  for file in files:
    
    # file path
    # filepath = os.path.join(root, file)
    filename = os.path.join(root, file)
    print(os.path.join(root, file))
    
    # file size in bytes
    filesize = os.path.getsize(filename)
    print(os.path.getsize(filename))
    
    # modified time
    date_time = time.ctime( os.path.getmtime(filename))
    print(os.path.getmtime(filename))
    print(date_time)
    
    # get md5 hash values
    md5digest, md5hexdigest =  hashFunction(filename)
    
    # write to csv
    csv_data = [file, md5hexdigest, filesize,md5digest, date_time]
    writer.writerow(csv_data)
    
    # rename files
    oldext = os.path.splitext(file)[1]
    newfilename = md5hexdigest + oldext
    newfilename = os.path.join(root, newfilename)
    print(newfilename)
    os.rename(filename, newfilename)

csv_file.close()

print("End" + os.getcwd())
os.listdir()     
os.getcwd()



# with open(csv_file, mode='r') as csv_file:
#     csv_reader = csv.DictReader(csv_file)
#     line_count = 0
#     for row in csv_reader:
#         if line_count == 0:
#             print(f'Column names are {", ".join(row)}')
#             line_count += 1
#        # print(f'\t{row["name"]} works in the {row["department"]} department, and was born in {row["birthday month"]}.')
#         line_count += 1
#     print(f'Processed {line_count} lines.')

# """
# def main():

#   h = hashlib.md5(file)
#   output = h.hexdigest()
#   os.rename( file, output)

# if __name__ == '__main__':
#   main()      
# """

