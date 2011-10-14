from fabric.api import *

env.key_filename = ["/home/scj7t4/.ssh/id_rsa"]

BOOST_ROOT = '~/boost'

def host_type():
    run('uname -s')

def change_and_check():
    with cd('/tmp'):
        run('pwd')

def pull_checkout(branch='master'):
    """
    This command will checkout a branch and run it. It is meant
    to be run on nodes which clone from the main one, not the 
    one that interfaces directly with github.
    """
    with cd('~/FREEDM/Broker/src'):
        result = run('git branch | grep %s' % branch)
        if result.return_code != 0:
            result = run('git branch -r | grep %s' % branch)
            if result.return_code != 0:
                run('git checkout --track -b %s origin/%s' % (branch,branch))
        else:
            run('git checkout %s' % branch)
        run('git pull')
 
def build():
    with cd('~/FREEDM/Broker'):
        run("BOOST_ROOT='%s' cmake ." % BOOST_ROOT)
    with cd('~/FREEDM/Broker/src'):
        run("BOOST_ROOT='%s' make" % BOOST_ROOT)


