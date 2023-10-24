#!/usr/bin/python
import os


def Virtual_Box_Setup():
    arg1 = sys.argv[1] # User input which should be setup_VM, and will probably expand it later on to a better the rich-man VM AKA VMWare
    if arg1 == 'setup_VM':
        print('\n================ Creating and Setting Up Puhaa-OS In VM ================\n')
        os.system('VBoxManage storageattach Puhaa-OS --storagectl AHCI --port 0 --medium none --device 0')
        os.system('VBoxManage closemedium disk_images/Puhaa-OS.vdi')
        os.system('VBoxManage unregister --delete Puhaa-OS')
        os.system('VBoxManage createvm --name Puhaa-OS --register')
        os.system('VBoxManage storagectl Puhaa-OS --name AHCI --add sata --controller IntelAhci')
        os.system('VBoxManage modifyvm Puhaa-OS --hpet on')
        os.system('VBoxManage modifyvm Puhaa-OS --memory 4096')
        os.system('VBoxManage modifyvm Puhaa-OS --vram 128')
        os.system('VBoxManage modifyvm Puhaa-OS --bioslogofadein off')
        os.system('VBoxManage modifyvm Puhaa-OS --bioslogofadeout off')
        os.system('VBoxManage modifyvm Puhaa-OS --bioslogodisplaytime 0')
        os.system('VBoxManage modifyvm Puhaa-OS --biosbootmenu disabled')
        os.system('VBoxManage setextradata Puhaa-OS \'CustomVideoMode1\' \'2560x1440x32'', dont_quote = true')
        os.system('VBoxManage setextradata Puhaa-OS \'CustomVideoMode1\' \'1920x1080x32'', dont_quote = true')
        os.system('VBoxManage setextradata Puhaa-OS GUI/MaxGuestResolution any')
        os.system('VBoxManage setextradata Puhaa-OS GUI/DefaultCloseAction Poweroff')
        return
    if arg1 == 'run_VM':
        print('\n================ Running Puhaa-OS VM ================\n')
        os.system('VBoxManage controlvm Puhaa-OS poweroff')
        os.system('VBoxManage storageattach Puhaa-OS --storagectl AHCI --port 0 --medium none --device 0')
        os.system('VBoxManage closemedium disk_images/Puhaa-OS.vdi')
        os.system('VBoxManage storageattach Puhaa-OS --storagectl AHCI --port 0 --medium disk_images/Puhaa-OS.vdi --device 0 --type hdd')
        os.system('VBoxManage startvm Puhaa-OS -E VBOX_GUI_DBG_ENABLED=true')
        return

if __name__ == "__main__":
    Virtual_Box_Setup()