import os
import json
import shutil
import tkinter as tk
from sys import stdout as std
from tkinter import filedialog
from cprint import cprint,cconvert

def error(msg):
    cprint(f"[R]Quit: "+msg)
    input('Press enter to continue')
    exit()

def ask_folder():
    root = tk.Tk()
    root.withdraw()

    folder = filedialog.askdirectory()

    return folder if folder != '' else False

def check_dir(dir):
    return os.path.isdir(dir)

###############
#    SETUP    #
###############

steam_root = (os.environ["ProgramFiles(x86)"] + "/Steam").replace("\\","/")

#Check root dir
if not check_dir(steam_root): 
    cprint(f"[R]Steam directory not found. Please locate the steam folder")
    steam_root = ask_folder()
    if not steam_root: error("Please locate the Steam folder.")

cprint(f"[GR]Steam directory found. [G]({steam_root})")

#Check steamVR directory
steam_vr = steam_root+'/steamapps/common/SteamVR'
if check_dir(steam_vr):
    cprint(f"[R]SteamVR not found at [G]{steam_vr}[R].\n\n[Y]Is SteamVR installed?")
    
    #Ask user if installed
    if not input('(Yes or No): ').lower() in ['y','yes','ye']:
        error(f"Please install SteamVR before running this.")
    
    #Ask user for directory
    cprint(f"\n[Y]Please select the steamvr folder")
    steam_vr = ask_folder()

    #Check if folder exists
    if not steam_vr: error("Please select the SteamVR folder")

cprint(f"[GR]SteamVR directory located. [G]({steam_vr})")

config = steam_root + "/config/steamvr.vrsettings"
if not os.path.isfile(config): error(f'Could not find config file. [G]({config})')

with open(config) as f:
    config_data = json.load(f)

print('-'*100)

################
# Main install #
################

def write(msg):
    std.write(f'\r'+cconvert(msg))
    std.flush()

#Check if driver already installed
if not os.path.isdir(steam_vr+'/drivers/apriltagtrackers'):
    write(f"[Y]Moving driver files to [G]{steam_vr}/drivers...")

    #Move folder to drivers folder
    shutil.move('apriltagtrackers',steam_vr+'/drivers')
    cprint(f"[GR]Done!\n")
else:
    cprint(f"[GR]Driver files already installed, skipping\n")


#Check if already activated
if not 'activateMultipleDrivers' in config_data['steamvr'] or config_data['steamvr']['activateMultipleDrivers'] == False:
    write(f"[Y]Activating multiple drivers in SteamVR config...")
    
    #change config
    config_data['steamvr']['activateMultipleDrivers'] = True

    #save data
    with open(config,'w') as f:
        json.dump(config_data,f,sort_keys=True, indent=4)

    cprint(f"[GR]Done!\n")
else:
    cprint(f"[GR]Multiple drivers already activated, skipping\n")

cprint(f"[L]Driver succesfully installed\n")

print('-'*100)

input('\nPress enter to continue')