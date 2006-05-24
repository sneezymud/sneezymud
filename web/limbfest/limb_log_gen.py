#!/home/maror/bin/python2
import glob, os, sys

limb_path = "/mud/web/limbfest/logs/"
play_path = "/mud/prod/lib/logs/"
index_file = limb_path + "logcurrent.index"
logcurrent = "logcurrent"

def limb_parse(file, index = 0L):
  inF = open(play_path + file)
  outF = open(limb_path + file, "a")
  inF.seek(index)
  for line in inF:
    if "chopped by" in line:
      outF.write(line)
  newindex = inF.tell()
  inF.close()
  outF.close()
  return newindex
      
play_logs = glob.glob(play_path + "log*")
limb_logs = glob.glob(limb_path + "log*")
play_log_base = [os.path.basename(file) for file in play_logs]
limb_logs_base = [os.path.basename(file) for file in limb_logs]

for file in play_log_base:
  if file in limb_logs_base:
    continue
  # if there is a new file, then logcurrent is new too
  try:
    os.unlink(index_file)
  except:
    pass
  try:
    os.unlink(limb_path + logcurrent)
  except:
    pass
  limb_parse(file)

# deal with logcurrent
try:
  indexF = open(index_file)
  index = int(indexF.readline())
  indexF.close()
except:
  index = 0

newindex = limb_parse(logcurrent, index)
indexF = open(index_file, "w")
indexF.write("%s" % newindex)
indexF.close()
    

sys.exit(1)
