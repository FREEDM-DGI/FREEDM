"""
@file         templates.py

@author       Stephen Jackson <scj7t4@mst.edu>

@project      FREEDM DGI

@description  Generates the program outputs into the template files.

These source code files were created at Missouri University of Science and
Technology, and are intended for use in teaching or research. They may be
freely copied, modified, and redistributed as long as modified versions are
clearly marked as such and this notice is not removed. Neither the authors
nor Missouri S&T make any warranty, express or implied, nor assume any legal
responsibility for the accuracy, completeness, or usefulness of these files
or any information distributed with these files.

Suggested modifications or questions about these files can be directed to
Dr. Bruce McMillin, Department of Computer Science, Missouri University of
Science and Technology, Rolla, MO 65409 <ff@mst.edu>.
"""

from string import Template

def load_template(path):
    f = open(path)
    t = Template(f.read())
    f.close()
    return t

def cfg_file(output,generated):
    t_base = load_template('templates/cfg_base.tpl')
    t_parameter = load_template('templates/cfg_parameter.tpl')
    f = open(output,'w')
    p_block = ""
    for (parameter,value) in generated.iteritems():
        p_block += t_parameter.substitute({'parameter':parameter,'value':value})
    f.write(t_base.substitute({'parameter_block':p_block}))
    f.close()

def hpp_file(output,generated):
    t_base = load_template('templates/hpp_base.tpl')
    t_parameter = load_template('templates/hpp_parameter.tpl')
    f = open(output,'w')
    p_block = ""
    for (parameter,value) in generated.iteritems():
        p_block += t_parameter.substitute({'parameter':parameter,'value':value})
    f.write(t_base.substitute({'parameter_block':p_block}))
    f.close()

def cpp_file(output,generated):
    t_base = load_template('templates/cpp_base.tpl')
    t_parameter = load_template('templates/cpp_parameter.tpl')
    t_parameter2 = load_template('templates/cpp_parameter2.tpl')
    f = open(output,'w')
    p_block = ""
    p_block2 = ""
    for (parameter,value) in generated.iteritems():
        p_block += t_parameter.substitute({'parameter':parameter,'value':value})
        p_block2 += t_parameter2.substitute({'parameter':parameter,'value':value})
    f.write(t_base.substitute({'parameter_block':p_block,'parameter_block2':p_block2}))
    f.close()
