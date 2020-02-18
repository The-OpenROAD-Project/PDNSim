pipeline {
  agent any
   wrappers {
        preBuildCleanup { // Clean before build
            includePattern('/home/openroad/.jenkins/workspace/PDNSim_master/build')
            deleteDirectories()
            cleanupParameter('CLEANUP')
        }
   }
  stages {
    stage('Build') {
      steps {
        sh './jenkins/build.sh'
      }
    }
    stage('Test') {
      parallel {
        stage('Python Tests') {
          steps {
            sh './jenkins/test-py.sh'
          }
        }
        stage('TCL Tests') {
          steps {
            sh './jenkins/test-tcl.sh'
          }
        }
      }
    }
  }
}
