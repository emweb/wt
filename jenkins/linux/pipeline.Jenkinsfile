#!/usr/bin/env groovy

def user_id
def user_name
def group_id
def group_name
def container_ccache_dir
def host_ccache_dir

def thread_count = 10

// Remember the last Gitlab stage that was touched
// This is used to ensure that pipelines are always properly "closed" in Gitlab
// Such that they do not indefinitely remain in "Running".
// This is a "hack" necessary in declarative pipelines, since the "gitlabBuilds"
// command does not correctly push all initial stages.
def last_gitlab_stage

node('docker') {
    user_id = sh(returnStdout: true, script: 'id -u').trim()
    user_name = sh(returnStdout: true, script: 'id -un').trim()
    group_id = sh(returnStdout: true, script: 'id -g').trim()
    group_name = sh(returnStdout: true, script: 'id -gn').trim()
    container_ccache_dir = "/home/${user_name}/.ccache"
    host_ccache_dir = "/local/home/${user_name}/.ccache"
}

def wt_configure(Map args) {
    sh """cmake .. \
            -DCMAKE_BUILD_TYPE=RelWithDebInfo \
            -DCMAKE_C_COMPILER_LAUNCHER=ccache \
            -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
            -DBUILD_EXAMPLES=ON \
            -DBUILD_TESTS=ON \
            -DCONNECTOR_FCGI=OFF \
            -DCONNECTOR_HTTP=ON \
            -DDEBUG=ON \
            -DEXAMPLES_CONNECTOR=wthttp \
            -DENABLE_HARU=ON \
            -DHARU_PREFIX=/opt/haru \
            -DENABLE_PANGO=ON \
            -DWT_WRASTERIMAGE_IMPLEMENTATION=GraphicsMagick \
            -DENABLE_QT4=OFF \
            -DENABLE_QT5=OFF \
            -DENABLE_QT6=ON \
            -DENABLE_SQLITE=ON \
            -DUSE_SYSTEM_SQLITE3=ON \
            -DENABLE_POSTGRES=OFF \
            -DENABLE_FIREBIRD=OFF \
            -DENABLE_MYSQL=OFF \
            -DENABLE_MSSQLSERVER=OFF \
            -DENABLE_OPENGL=ON \
            -DENABLE_SAML=ON \
            -DENABLE_SSL=ON \
            -DHTTP_WITH_ZLIB=ON \
            -DSHARED_LIBS=ON \
            -DMULTI_THREADED=${args.mt}"""
}

pipeline {
    environment {
        EMAIL = credentials('wt-dev-mail')
    }
    options {
        buildDiscarder logRotator(numToKeepStr: '20')
        disableConcurrentBuilds abortPrevious: true

        gitLabConnection('Gitlab')
        // See https://github.com/jenkinsci/gitlab-plugin/issues/1028
        // These stages are not all initially pushed to Gitlab, which they SHOULD be.
        // As such, Gitlab will consider the whole pipeline a success if the latest stage it sees is successful.
        // Only after this "success" will the new 'running' stage be seen.
        // "Set to merge on pipeline success" then suffers from prematurely merging the branch.
        gitlabBuilds(builds: ['Overarching Pipeline', 'Format JavaScript', 'Build - Single Threaded', 'Build - Multi Threaded', 'Tests', 'Tests - Sqlite3', 'Wt Port - Checkout', 'Wt Port - Config', 'Wt Port - TinyMCE', 'Wt Port - CNOR', 'Wt Port - Java Build', 'Wt Port - Java Test'])
    }
    // Start without an agent, and define the agent per each stage.
    // This is done to ensure that wt and wt-port use different dockerfiles.
    agent none
    stages {
        stage('wt') {
            agent {
                dockerfile {
                    label 'docker'
                    dir 'jenkins/linux'
                    filename 'pipeline.Dockerfile'
                    args "--env CCACHE_DIR=${container_ccache_dir} --env CCACHE_MAXSIZE=20G --volume ${host_ccache_dir}:${container_ccache_dir}:z"
                    additionalBuildArgs """--build-arg USER_ID=${user_id} \
                                           --build-arg USER_NAME=${user_name} \
                                           --build-arg GROUP_ID=${group_id} \
                                           --build-arg GROUP_NAME=${group_name} \
                                           --build-arg THREAD_COUNT=${thread_count}"""
                }
            }
            stages {
                stage('Check JS') {
                    stages {
                        stage('pnpm install') {
                            steps {
                                script {
                                    last_gitlab_stage = "Format JavaScript"
                                }
                                updateGitlabCommitStatus name: 'Overarching Pipeline', state: 'running'
                                updateGitlabCommitStatus name: 'Format JavaScript', state: 'running'
                                dir('src/js') {
                                    sh '''#!/bin/bash
                                      export PNPM_HOME="${HOME}/.local/share/pnpm"
                                      export PATH="${PNPM_HOME}:${PATH}"
                                      pnpm install
                                    '''
                                }
                            }
                        }
                        stage('Check formatting') {
                            steps {
                                dir('src/js') {
                                    sh '''#!/bin/bash
                                      export PNPM_HOME="${HOME}/.local/share/pnpm"
                                      export PATH="${PNPM_HOME}:${PATH}"
                                      pnpm run checkfmt
                                    '''
                                }
                            }
                        }
                        stage('Linting') {
                            steps {
                                dir('src/js') {
                                    sh '''#!/bin/bash
                                      export PNPM_HOME="${HOME}/.local/share/pnpm"
                                      export PATH="${PNPM_HOME}:${PATH}"
                                      pnpm run lint-junit
                                    '''
                                }
                            }
                        }
                    }
                    post {
                        failure {
                            updateGitlabCommitStatus name: 'Format JavaScript', state: 'failed'
                        }
                        success {
                            updateGitlabCommitStatus name: 'Format JavaScript', state: 'success'
                        }
                        aborted {
                            updateGitlabCommitStatus name: 'Format JavaScript', state: 'canceled'
                        }
                    }
                }
                stage('Building application - Single threaded') {
                    steps {
                        script {
                            last_gitlab_stage = "Build - Single Threaded"
                        }
                        updateGitlabCommitStatus name: 'Build - Single Threaded', state: 'running'
                        dir('build-st') {
                            wt_configure(mt: 'OFF')
                            sh "make -k -j${thread_count}"
                            sh "make -C examples -k -j${thread_count}"
                        }
                    }
                    post {
                        failure {
                            updateGitlabCommitStatus name: 'Build - Single Threaded', state: 'failed'
                        }
                        success {
                            updateGitlabCommitStatus name: 'Build - Single Threaded', state: 'success'
                        }
                        aborted {
                            updateGitlabCommitStatus name: 'Build - Single Threaded', state: 'canceled'
                        }
                    }
                }
                stage('Building application - Multi threaded') {
                    steps {
                        script {
                            last_gitlab_stage = "Build - Multi Threaded"
                        }
                        updateGitlabCommitStatus name: 'Build - Multi Threaded', state: 'running'
                        dir('build-mt') {
                            wt_configure(mt: 'ON')
                            sh "make -k -j${thread_count}"
                            sh "make -C examples -k -j${thread_count}"
                        }
                    }
                    post {
                        failure {
                            updateGitlabCommitStatus name: 'Build - Multi Threaded', state: 'failed'
                        }
                        success {
                            updateGitlabCommitStatus name: 'Build - Multi Threaded', state: 'success'
                        }
                        aborted {
                            updateGitlabCommitStatus name: 'Build - Multi Threaded', state: 'canceled'
                        }
                    }
                }
                stage('Tests') {
                    steps {
                        script {
                            last_gitlab_stage = "Tests"
                        }
                        updateGitlabCommitStatus name: 'Tests', state: 'running'
                        dir('test') {
                            warnError('st test.wt failed') {
                                sh "../build-st/test/test.wt --log_format=JUNIT --log_level=all --log_sink=${env.WORKSPACE}/st_test_log.xml"
                            }
                            // No st test.http - this requires multi threading
                            warnError('st thirdpartytest.wt failed') {
                                sh "../build-st/test/thirdpartytest.wt --log_format=JUNIT --log_level=all --log_sink=${env.WORKSPACE}/st_thirdparty_test_log.xml"
                            }
                        }
                        dir('test') {
                            warnError('mt test.wt failed') {
                                sh "../build-mt/test/test.wt --log_format=JUNIT --log_level=all --log_sink=${env.WORKSPACE}/mt_test_log.xml"
                            }
                            warnError('mt test.http failed') {
                                sh "../build-mt/test/test.http --log_format=JUNIT --log_level=all --log_sink=${env.WORKSPACE}/mt_http_test_log.xml"
                            }
                            warnError('mt thirdpartytest.wt failed') {
                                sh "../build-mt/test/thirdpartytest.wt --log_format=JUNIT --log_level=all --log_sink=${env.WORKSPACE}/mt_thirdparty_test_log.xml"
                            }
                        }
                    }
                    post {
                        failure {
                            updateGitlabCommitStatus name: 'Tests', state: 'failed'
                        }
                        success {
                            updateGitlabCommitStatus name: 'Tests', state: 'success'
                        }
                        aborted {
                            updateGitlabCommitStatus name: 'Tests', state: 'canceled'
                        }
                        unstable {
                            updateGitlabCommitStatus name: 'Tests', state: 'failed'
                        }
                    }
                }
                stage('Test SQLite3') {
                    steps {
                        script {
                            last_gitlab_stage = "Tests - Sqlite3"
                        }
                        updateGitlabCommitStatus name: 'Tests - Sqlite3', state: 'running'
                        dir('test') {
                            warnError('st test.sqlite3 failed') {
                                sh "../build-st/test/test.sqlite3 --report_level=detailed --log_format=JUNIT --log_level=all --log_sink=${env.WORKSPACE}/st_sqlite3_test_log.xml"
                            }
                        }
                        dir('test') {
                            warnError('mt test.sqlite3 failed') {
                                sh "../build-mt/test/test.sqlite3 --report_level=detailed --log_format=JUNIT --log_level=all --log_sink=${env.WORKSPACE}/mt_sqlite3_test_log.xml"
                            }
                        }
                    }
                    post {
                        failure {
                            updateGitlabCommitStatus name: 'Tests - Sqlite3', state: 'failed'
                        }
                        success {
                            updateGitlabCommitStatus name: 'Tests - Sqlite3', state: 'success'
                        }
                        aborted {
                            updateGitlabCommitStatus name: 'Tests - Sqlite3', state: 'canceled'
                        }
                        unstable {
                            updateGitlabCommitStatus name: 'Tests - Sqlite3', state: 'failed'
                        }
                    }
                }
            }
            post {
                always {
                    junit allowEmptyResults: true, testResults: '*_test_log.xml'
                    script {
                        // Specific case to detect superseded builds.
                        if (currentBuild.currentResult == 'NOT_BUILT') {
                          updateGitlabCommitStatus name: 'Overarching Pipeline', state: 'canceled'
                          updateGitlabCommitStatus name: "${last_gitlab_stage}", state: 'canceled'
                        }
                    }
                }
                cleanup {
                    cleanWs()
                }
                aborted {
                    updateGitlabCommitStatus name: 'Overarching Pipeline', state: 'canceled'
                }
                failure {
                    updateGitlabCommitStatus name: 'Overarching Pipeline', state: 'failed'
                    mail to: env.EMAIL,
                         subject: "Failed Pipeline (wt step): ${currentBuild.fullDisplayName}",
                         body: "Something is wrong with ${env.BUILD_URL}"
                }
                unstable {
                    updateGitlabCommitStatus name: 'Overarching Pipeline', state: 'failed'
                    mail to: env.EMAIL,
                         subject: "Unstable Pipeline (wt step): ${currentBuild.fullDisplayName}",
                         body: "Something is wrong with ${env.BUILD_URL}"
                }
            }
        }
        stage('wt-port') {
            agent {
                dockerfile {
                    label 'docker'
                    dir 'jenkins/linux'
                    filename 'pipeline-port.Dockerfile'
                    additionalBuildArgs """--build-arg USER_ID=${user_id} \
                                           --build-arg USER_NAME=${user_name} \
                                           --build-arg GROUP_ID=${group_id} \
                                           --build-arg GROUP_NAME=${group_name}"""
                }
            }
            stages {
                stage('Wt-port Checkout') {
                    steps {
                        script {
                            last_gitlab_stage = "Wt Port - Checkout"
                        }
                        updateGitlabCommitStatus name: 'Wt Port - Checkout', state: 'running'
                        script {
                            // Checks out master by default.
                            def output = sh(returnStatus: true, script: "git clone https://git.leuven.emweb.be/gitlab/emweb/wt-port.git");
                            dir ('wt-port') {
                                // Avoid recognizing a remote branch contain "null" as a potential target.
                                if ("${env.BRANCH_NAME}" != "null") {
                                    def remoteBranches = sh(returnStdout: true, script: "git branch -r");

                                    // Check if the current wt branch has a wt-port counterpart.
                                    // Check out the counterpart if it exists.
                                    if (remoteBranches.contains("${env.BRANCH_NAME}")) {
                                        sh "git checkout ${env.BRANCH_NAME}"
                                    }

                                    // Check based on ticket number.
                                    def currentTicketNumber = "${env.BRANCH_NAME}".split('/')[0].trim();
                                    def branches = remoteBranches.tokenize('\n');
                                    // Find whether a branch exists that starts with a ticket number,
                                    // being the same as the current branch's ticket number.
                                    // `it` being the implicit element
                                    // Branch format: origin/{ticket}/{description}
                                    def foundBranch = branches.find { it.trim().split('/')[1].trim() == currentTicketNumber };
                                    if (foundBranch != null) {
                                        sh "git checkout ${foundBranch}"
                                    }
                                }
                            }
                        }
                    }
                    post {
                        failure {
                            updateGitlabCommitStatus name: 'Wt Port - Checkout', state: 'failed'
                        }
                        success {
                            updateGitlabCommitStatus name: 'Wt Port - Checkout', state: 'success'
                        }
                        aborted {
                            updateGitlabCommitStatus name: 'Wt Port - Checkout', state: 'canceled'
                        }
                    }
                }
                stage('Config') {
                    steps {
                        script {
                            last_gitlab_stage = "Wt Port - Config"
                        }
                        updateGitlabCommitStatus name: 'Wt Port - Config', state: 'running'
                        dir('wt-port/java') {
                            sh """cat > Config << EOF
WTDIR=${env.WORKSPACE}
JAVA=/usr/bin/java
WT_PORT=${env.WORKSPACE}/wt-port
JWT_GIT=${env.WORKSPACE}/jwt
EOF"""
                        }
                    }
                    post {
                        failure {
                            updateGitlabCommitStatus name: 'Wt Port - Config', state: 'failed'
                        }
                        success {
                            updateGitlabCommitStatus name: 'Wt Port - Config', state: 'success'
                        }
                        aborted {
                            updateGitlabCommitStatus name: 'Wt Port - Config', state: 'canceled'
                        }
                    }
                }
                stage('CNOR') {
                    steps {
                        script {
                            last_gitlab_stage = "Wt Port - CNOR"
                        }
                        updateGitlabCommitStatus name: 'Wt Port - CNOR', state: 'running'
                        // While this CAN be build with multiple threads it is generally a bad idea, as it is likely to fail at least once then.
                        // The issue is that some of the grammar is build on-demand, and then used as an include.
                        // If a file that depends on this grammar is being attempted to compile before the grammar is ready, this will
                        // result in an inclusion failure. Thus it's generally "safer" to work on a single thread.
                        dir('wt-port/oink/elsa-stack') {
                            sh './configure'
                            sh 'make'
                        }
                    }
                    post {
                        failure {
                            updateGitlabCommitStatus name: 'Wt Port - CNOR', state: 'failed'
                        }
                        success {
                            updateGitlabCommitStatus name: 'Wt Port - CNOR', state: 'success'
                        }
                        aborted {
                            updateGitlabCommitStatus name: 'Wt Port - CNOR', state: 'canceled'
                        }
                    }
                }
                // This step is necessary since the Makefile requires it.
                // This ought to be moved to a different step.
                stage('Copy TinyMCE') {
                    steps {
                        script {
                            last_gitlab_stage = "Wt Port - TinyMCE"
                        }
                        updateGitlabCommitStatus name: 'Wt Port - TinyMCE', state: 'running'
                        sh "cp -r /opt/tinymce/3/tinymce/jscripts/tiny_mce ${env.WORKSPACE}/resources/"
                        sh "cp -r /opt/tinymce/4/tinymce/js/tinymce ${env.WORKSPACE}/resources/"
                        sh "mkdir -p ${env.WORKSPACE}/resources/tinymce6"
                        sh "cp -r /opt/tinymce/6/tinymce/js/tinymce/* ${env.WORKSPACE}/resources/tinymce6/"
                    }
                    post {
                        failure {
                            updateGitlabCommitStatus name: 'Wt Port - TinyMCE', state: 'failed'
                        }
                        success {
                            updateGitlabCommitStatus name: 'Wt Port - TinyMCE', state: 'success'
                        }
                        aborted {
                            updateGitlabCommitStatus name: 'Wt Port - TinyMCE', state: 'canceled'
                        }
                    }
                }
                stage('Clean-dist') {
                    steps {
                        script {
                            last_gitlab_stage = "Wt Port - Java Build"
                        }
                        updateGitlabCommitStatus name: 'Wt Port - Java Build', state: 'running'
                        dir('wt-port/java') {
                            sh "make clean-dist -j${thread_count}"
                            dir('examples') {
                                sh 'ant'
                            }
                        }
                    }
                    post {
                        failure {
                            updateGitlabCommitStatus name: 'Wt Port - Java Build', state: 'failed'
                        }
                        success {
                            updateGitlabCommitStatus name: 'Wt Port - Java Build', state: 'success'
                        }
                        aborted {
                            updateGitlabCommitStatus name: 'Wt Port - Java Build', state: 'canceled'
                        }
                    }
                }
                stage('Test') {
                    steps {
                        script {
                            last_gitlab_stage = "Wt Port - Java Test"
                        }
                        updateGitlabCommitStatus name: 'Wt Port - Java Test', state: 'running'
                        dir('wt-port/java') {
                            warnError('tests failed') {
                                sh 'ant test'
                            }
                        }
                    }
                    post {
                        failure {
                            updateGitlabCommitStatus name: 'Wt Port - Java Test', state: 'failed'
                        }
                        success {
                            updateGitlabCommitStatus name: 'Wt Port - Java Test', state: 'success'
                        }
                        aborted {
                            updateGitlabCommitStatus name: 'Wt Port - Java Test', state: 'canceled'
                        }
                        unstable {
                            updateGitlabCommitStatus name: 'Wt Port - Java Test', state: 'failed'
                        }
                    }
                }
            }
            post {
                success {
                    updateGitlabCommitStatus name: 'Overarching Pipeline', state: 'success'
                    archiveArtifacts artifacts: 'wt-port/java/dist/*.jar,wt-port/java/*.pom', fingerprint: true
                }
                aborted {
                    updateGitlabCommitStatus name: 'Overarching Pipeline', state: 'canceled'
                }
                always {
                    junit allowEmptyResults: true, testResults: 'wt-port/java/report/TEST-eu.webtoolkit.jwt*.xml'
                    script {
                        // Specific case to detect superseded builds.
                        if (currentBuild.currentResult == 'NOT_BUILT') {
                          updateGitlabCommitStatus name: 'Overarching Pipeline', state: 'canceled'
                          updateGitlabCommitStatus name: "${last_gitlab_stage}", state: 'canceled'
                        }
                    }
                }
                cleanup {
                    cleanWs()
                }
                failure {
                    updateGitlabCommitStatus name: 'Overarching Pipeline', state: 'failed'
                    mail to: env.EMAIL,
                         subject: "Failed Pipeline (wt-port step): ${currentBuild.fullDisplayName}",
                         body: "Something is wrong with ${env.BUILD_URL}"
                }
                unstable {
                    updateGitlabCommitStatus name: 'Overarching Pipeline', state: 'failed'
                    mail to: env.EMAIL,
                         subject: "Unstable Pipeline (wt-port step): ${currentBuild.fullDisplayName}",
                         body: "Something is wrong with ${env.BUILD_URL}"
                }
            }
        }
    }
}
