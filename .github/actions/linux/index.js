const core = require("@actions/core");
const github = require("@actions/github");
const exec = require("@actions/exec");
const artifact = require("@actions/artifact");

const makefile = core.getInput("makefile");
const architecture = core.getInput("architecture");

const artifactClient = artifact.create();
const artifactName = `linux-${architecture}`;

async function run() {
  try {

    console.log( await exec.exec("ls") )
    process.env.ARCHITECTURE = architecture;
    if(makefile === 'Makefile') {
    await exec.exec("sudo", [
      `add-apt-repository`, `-y`, `ppa:ubuntu-toolchain-r/test`,
    ]);
    await exec.exec("sudo", [`apt-get`, `update`]);
    await exec.exec("sudo", [
      `apt-get`, `install`, `libx264-dev`, `gcc-10`, `g++-10`, `-y`,
    ]);
    await exec.exec("sudo", [
      `update-alternatives`, `--install`, `/usr/bin/gcc`, `gcc`, `/usr/bin/gcc-10`, `10`,
    ]);
    await exec.exec("sudo", [
      `update-alternatives`, `--install`, `/usr/bin/g++`, `g++`, `/usr/bin/g++-10`, `10`,
    ]);
  }

    await exec.exec("make", [
      `-f`, `${makefile}`, `exec`, `-C`, `PopH264.Linux/`,
    ]);

    const files = [
      `PopH264.Linux/PopH264-${architecture}.so`,
      `PopH264.Linux/PopH264TestApp-${architecture}`,
    ];
    
    const rootDirectory = ".";
    console.log( await exec.exec("ls", [buildDirectory]) )

    const options = {
      continueOnError: false,
    };
    const uploadResponse = await artifactClient.uploadArtifact(
      artifactName,
      files,
      rootDirectory,
      options
    );
  } catch (error) {
    core.setFailed(error.message);
  }
}

run();
