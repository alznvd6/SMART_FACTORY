$folders = @(
    "docs/requirements",
    "docs/architecture",
    "docs/uml",
    "docs/reports",

    "presentation/slides",
    "presentation/images",

    "subsystems/fire-system/docs",
    "subsystems/fire-system/hardware/schematics",
    "subsystems/fire-system/hardware/proteus",
    "subsystems/fire-system/hardware/sensors",
    "subsystems/fire-system/hardware/screenshots",
    "subsystems/fire-system/firmware/src",
    "subsystems/fire-system/firmware/include",
    "subsystems/fire-system/firmware/tests",

    "subsystems/machine-monitoring/docs",
    "subsystems/machine-monitoring/hardware/schematics",
    "subsystems/machine-monitoring/hardware/proteus",
    "subsystems/machine-monitoring/hardware/sensors",
    "subsystems/machine-monitoring/hardware/screenshots",
    "subsystems/machine-monitoring/firmware/src",
    "subsystems/machine-monitoring/firmware/include",
    "subsystems/machine-monitoring/firmware/tests",

    "subsystems/quality-control/docs",
    "subsystems/quality-control/hardware/schematics",
    "subsystems/quality-control/hardware/proteus",
    "subsystems/quality-control/hardware/sensors",
    "subsystems/quality-control/hardware/screenshots",
    "subsystems/quality-control/firmware/src",
    "subsystems/quality-control/firmware/include",
    "subsystems/quality-control/firmware/tests",

    "subsystems/workforce-management/docs",
    "subsystems/workforce-management/hardware/schematics",
    "subsystems/workforce-management/hardware/proteus",
    "subsystems/workforce-management/hardware/sensors",
    "subsystems/workforce-management/hardware/screenshots",
    "subsystems/workforce-management/firmware/src",
    "subsystems/workforce-management/firmware/include",
    "subsystems/workforce-management/firmware/tests",

    "shared/firmware/configs",
    "shared/firmware/utils",

    "dashboard/admin-panel",
    "dashboard/monitoring",
    "dashboard/reports",

    "mobile-app",

    "infrastructure/database",
    "infrastructure/notifications",
    "infrastructure/deployment"
)

foreach ($folder in $folders) {
    New-Item -ItemType Directory -Force -Path $folder | Out-Null
}
foreach ($folder in $folders) {
    New-Item -ItemType File `
        -Force `
        -Path (Join-Path $folder "placeholder.txt") | Out-Null
}

Write-Host "Project structure created successfully."