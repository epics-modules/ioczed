# soft scaler
dbLoadRecords("$(ASYN)/db/asynRecord.db","P=zzz:,R=asynScaler,PORT=scaler1Port,ADDR=0,OMAX=0,IMAX=0")
# drvScalerSoftConfigure(char *portName, int maxChans, char *pvTemplate)
drvScalerSoftConfigure("scaler1Port", 8, "zzz:scaler1:s%d")
dbLoadRecords("$(STD)/stdApp/Db/scaler.db","P=zzz:,S=scaler1,OUT=@asyn(scaler1Port 0 0),DTYP=Asyn Scaler,FREQ=10000000")
dbLoadRecords("$(STD)/stdApp/Db/scalerSoftCtrl.db","P=zzz:,Q=scaler1:,SCALER=zzz:scaler1")
