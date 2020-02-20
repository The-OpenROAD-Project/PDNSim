pipeline {
  agent any
   stages {
    stage('Build') {
      steps {
        sh './jenkins/build.sh'
      }
    }
    stage('Test') {
      parallel {
        stage('Tests') {
          steps {
            sh './jenkins/test.sh'
          }
        }
      }
    }
  }
}
