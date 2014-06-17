from fabric.api import run, local, env, cd, lcd, settings
from fabric.operations import put, get
from fabric.decorators import roles

env.password = "rootkoko"
env.roledefs = {
    'all': [
        'root@alarmpi.local',
        'root@alarmpi-2.local',
        'root@alarmpi-3.local',
    ],
}

env.skip_bad_hosts=True

@roles('all')
def setip_all(ip):
    setip(ip)

def setip(ip):
    run('rm /boot/recipient.txt')
    run('echo "' + ip + '" >> /boot/recipient.txt')

@roles('all')
def upgrade_all():
    upgrade()

def upgrade():
    run('sudo apt-get update')
    run('sudo apt-get upgrade')

@roles('leader')
def test_run():
    start()

@roles('all')
def deploy_reset_all():
    deploy()
    resetSettings();

def reboot():
    with settings(warn_only=True):
        run('reboot') 

@roles('all')
def reboot_all():
    reboot()
