<?php
/**
 * PHP Template.
 */

  include('adminapi.php');
  include('randomPassword.php');
  function importUsersToCompanies($anArray){
      global $lastError;
      $results = array();
      //remove comments added to the import file - they start with #
      $noCommentArray = array();
      foreach ($anArray as $record) {
          if ($record[0][0]!='#') {
              $noCommentArray[]=$record;
          }
      }
      $companies = getCompanyInformation($noCommentArray);
      foreach ($noCommentArray as $record) {
          $values = array();
          $values['name'] = $record[1];
          $password = $record[2];
          if ($password == ""){
              $password = randomPassword();
          }
          $values['password']=$password;
          $values['email'] = $record[1];
          $values['status'] = 'ActiveUser';
          $values['firstName'] = $record[3];
          $values['lastName'] = $record[4];
          $values['company'] = $record[0];
          $values['companyInfo']=$companies[$values['company']];
          $values['comment'] = $record[6];
          $values['sendEmail'] = (strtolower($record[7])=='true');
          $values['role'] = $record[5];
          if ($values['role']=='member' or strtolower($values['role'])=='regular user'){
              $values['role']='user';
          }
          $values['id']=createUser($values);
          if(!isset($values['id'])){
            $values['createUserError']=$lastError;  
            $values['id']=findUserByName($record[1]);
            $values['id']=$values['id']['id'];
          } else {
              $rs = updateUser($values['id'], $values);
              if (!$rs){
                  $values['createUserError']='Could not create password';
              }
          };
          $result[$record[1]]=$values;
      }
      foreach ($result as $user=>$values){
          global $lastError;
          $accountValues = array();
          $accountValues['userid']=$values['id'];
          $accountValues['orgid']=$values['companyInfo']['id'];
          $accountValues['userName']=$user;
          $accountValues['orgName']=$values['company'];
          $accountValues['status']='Invited';
          $accountValues['role']=$values['role'];
	  $accountValues['type']= 'user';
          if (isset($accountValues['orgid']) and isset($accountValues['userid'])){
                $accountValues['id'] = createAccount($accountValues);
          } else {
                $lastError = '<b>Error: </b>Cannot add '.$user.' to '. $values['company'].' because the company does not exist.  Please create the company first';
          }
          if(isset($accountValues['id'])){
              if ($values['sendEmail']) {
                  sendActivationRequest('',$values);
              }
          } else {
              $accountValues['createAccountError']=$lastError;
          }
          $result[$user]['accounts'][$accountValues['orgName']]=$accountValues;
      }
      return $result;
  }
  
  function getCompanyInformation($anArray){
      $companies = array();
      foreach($anArray as $record){
         $companies[$record[0]] = '';
      }
      foreach($companies as $key=>$value){
          $companies[$key] = findOrgByName($key);
      }
      return $companies;
  }
  
  $records = array();
  $fileName = $_FILES['importUserFile']['tmp_name'];
  if ($handle = fopen($fileName, "r")) {
      while ($contents = fgetcsv($handle, filesize($fileName),",")) {
          $records[] = $contents;
      }
      $result = importUsersToCompanies($records);
      include('importUsersResult.php');
      
  }
  
  
    
?>
