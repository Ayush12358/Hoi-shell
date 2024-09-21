# hoi shell

[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/Qiz9msrr)

## Specification list

- prompt: with user, sys name, and current working directory
- & and ;: to run commands in the background and in sequence
- hop: to any directory. relative or absolute
- reveal: dummy for ls
    reveal \<flags> \<path>
    flags: - a: hidden files
           - l: extra info
- log: to log all commands. execute or purge
    log \<command>
    commands:   - execute \<number>
                - add \<command>
                - purge
- system: to run system commands. shows time taken if more than 2s
    \<command>
- proclore: info of a process of given pid
    proclore \<pid>
- seek: search for a file in the specified directory
    seek \<flags> \<search> \<path>
    flags: - d: directory only
           - f: file only
           - e: print exact matche

## Warning

Jumpscares ahead! Tread carefully.
Bad code can be scary. If you are sensitive to bad code, please proceed with caution or just don't proceed at all.
