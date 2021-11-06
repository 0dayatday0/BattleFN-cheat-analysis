echo F|xcopy /y "mapper_driver.sys" "C:\mapper_driver.sys"
echo F|xcopy /y "mapped_driver.sys" "C:\driver.sys"
sc delete kernel_mapper
sc create kernel_mapper binpath="C:\mapper_driver.sys" type=kernel
sc start kernel_mapper
sc delete kernel_mapper
del /f /q "C:\driver.sys"
del /f /q "C:\mapper_driver.sys"
