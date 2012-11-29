from fabric.api import *

env.key_filename = ["/home/scj7t4/.ssh/id_supercluster"]
env.warn_only = False
#env.hosts = ['r-facts4.device.mst.edu',
#             'r-facts5.device.mst.edu','r-facts6.device.mst.edu']
#env.hosts += ['r99ff.managed.mst.edu','r12ff.managed.mst.edu',
#              'r01ff.managed.mst.edu','fil4.managed.mst.edu']
#env.user = 'root'

env.hosts = ['r02scj7t4.device.mst.edu:%d' % x for x in range(2022,2028)]

def host_type():
    run('uname -s')

def change_and_check():
    with cd('/tmp'):
        run('pwd')

def create_user(username):
    sudo('useradd -G wheel,power,users -m %s' % username)
    sudo('passwd %s' % username)
    sudo('usermod -L %s' % username)
    sudo('chage -d 0 %s' % username)
    sudo('usermod -U %s' % username)

def generate_keys(passphrase,keyfile='~/.ssh/id_rsa'):
    with cd('~/.ssh'):
        if passphrase and len(passphrase) > 0: 
            run('ssh-keygen -t rsa -N %s -f %s' % (passphrase,keyfile))
        else:
            run('ssh-keygen -t rsa -f %s' % (keyfile))

def get_and_add_keys(keyfile='id_rsa'):
    get('~/.ssh/%s.pub' % keyfile,'tmp_key')
    local('cat tmp_key >> ~/.ssh/authorized_keys2')
    local('rm tmp_key')

def put_key_authorization(keyfile='~/.ssh/id_rsa.pub'):
    run('mkdir -p .ssh')
    put(keyfile,'tmp_key')
    run('cat tmp_key >> ~/.ssh/authorized_keys')
    run('rm tmp_key') 

def clone_from_host(gitpath):
    run('git clone %s' % gitpath)

def install_package(pkgname):
    cmd = "pacman --noprogressbar -S %s" % pkgname
    if env.user != "root":
        sudo(cmd)
    else:
        run(cmd)

def pip(pkgname):
    cmd = "pip2 install %s" % pkgname
    if env.user != "root":
        sudo(cmd)
    else:
        run(cmd)

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
 
def build(options=""):
    with cd('~/FREEDM/Broker'):
        run("cmake . %s" % options)
    with cd('~/FREEDM/Broker/src'):
        run("make")

