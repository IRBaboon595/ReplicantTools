# ReplicantTools
Console app for catalogue sync

Second task from Veeam

This is a CONSOLE application.
It has four arguments:
1. Master Catalogue;
2. Slave Catalogue;
3. Log Catalogue;
4. Sync interval.

This soft synchronizes the Master directory and Slave directory each Sync interval miliseconds (Sync interval gets time in MSEC).
In any way - this soft is updating Slave directory by deleting all entries different from Masters Directorie's entries and copying Master's entries 
into Slave Directory.
Each action is logged in log.txt in 3rd arg catalogue.
