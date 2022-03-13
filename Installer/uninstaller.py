from os import path,environ
from json import load,dump
from shutil import copytree, rmtree
from sys import stdout as std
from tkinter import filedialog,Tk
from cprint import cprint

def error(msg):
    cprint(f"[R]Quit: "+msg)
    input('Press enter to continue')
    exit()

def ask_folder():
    root = Tk()
    root.withdraw()

    folder = filedialog.askdirectory()

    return folder if folder != '' else False

def check_dir(directory):
    return path.isdir(directory)

###############
#    SETUP    #
###############

steam_root = (environ["ProgramFiles(x86)"] + "/Steam").replace("\\","/")

#Check root dir
if not check_dir(steam_root): 
    cprint(f"[R]Steam directory not found. Please locate the steam folder")
    steam_root = ask_folder()
    if not steam_root: error("Please locate the Steam folder.")

cprint(f"[GR]Steam directory found. [G]({steam_root})")

#Check steamVR directory
steam_vr = steam_root+'/steamapps/common/SteamVR'
if not check_dir(steam_vr):
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
if not path.isfile(config): error(f'Could not find config file. [G]({config})')

with open(config) as f:
    config_data = load(f)

print('-'*100)

################
# Main install #
################


#Check if driver already installed
if not path.isdir(steam_vr+'/drivers/apriltagtrackers'):
    cprint(f"\n[R]Driver not installed, skipping...\n")

else:
    cprint(f"[GR]Driver files found, uninstalling...\n")
    rmtree(steam_vr+'/drivers/apriltagtrackers')
    cprint(f"[GR]Done!\n")


cprint(f"[L]Driver succesfully uninstalled\n")

print('-'*100)

input('\nPress enter to continue')