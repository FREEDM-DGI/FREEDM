from fabric.api import *

env.key_filename = ["/home/scj7t4/.ssh/id_supercluster"]
env.warn_only = False
env.hosts = ['r-facts3.device.mst.edu','r-facts4.device.mst.edu',
             'r-facts5.device.mst.edu','r-facts6.device.mst.edu']

def host_type():
    run('uname -s')

def change_and_check():
    with cd('/tmp'):
        run('pwd')

def create_user(username):
    sudo('useradd -G adm,dialout,cdrom,plugdev,lpadmin,sambashare -p powerpuff %s' % username)
    sudo('usermod -L %s' % username)
    sudo('chage -d 0' % username)
    sudo('usermod -U %s' % username)

def generate_keys(passphrase,keyfile='~/.ssh/id_rsa'):
    with cd('~/.ssh'):
        run('ssh-keygen -t rsa -N %s -f %s' % (passphrase,keyfile))

def get_and_add_keys(keyfile='id_rsa'):
    get('~/.ssh/%s.pub' % keyfile,'tmp_key')
    local('cat tmp_key >> ~/.ssh/authorized_keys2')
    local('rm tmp_key')

def clone_from_host(gitpath):
    run('git clone %s' % gitpath)

def install_package(pkgname):
    sudo("apt-get install %s" % pkgname)

def put_file(local,remote):
    put(local,remote)

def build_boost():
    with cd("~/boost"):
        sudo('./bootstrap.sh')
        sudo('./bjam install')

def pull_checkout(branch='master'):
    with cd('~/FREEDM'):
        result = run('git branch | grep %s' % branch)
        if result.return_code != 0:
            result = run('git branch -r | grep %s' % branch)
            if result.return_code != 0:
                run('git checkout --track -b %s origin/%s' % (branch,branch))
            else:
                print "Couldn't get into the specified branch! (Not Found)"
                exit(1)
        else:
            run('git checkout %s' % branch)
        run('git pull')
 
def build():
    with cd('~/FREEDM/Broker'):
        run("cmake .")
    with cd('~/FREEDM/Broker/src'):
        run("make")

