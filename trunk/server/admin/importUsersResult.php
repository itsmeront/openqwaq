<?php include('header.php'); ?>
<body id="edit_user" class="users">
<?php make_navbar('Users'); ?>

<div id="body" class="wrap">
  <h3>Import Users</h3>

  <form enctype="multipart/form-data" name="importUsersForm" method="POST" action="importUsers.php">
    <ul>
      <li>
        <label class="required">Import User File:</label>
        <input name="importUserFile" type="file">
      </li>
    </ul>
    
    <table style="border: 0px none ;">
    <tr><td style="background: white ;">
    <div class="submit button">
      <input value="Import" type="submit" class="submit">
    </div>
    </td>
    </tr>
    </table>
    <div class="sub_header"> 
        User Import Results
    </div> 
        <table class="alternate">
        <tr>
          <th>
              User
          </th>
          <th>
              System User
          </th>
          <th>
              System Account
          </th>
        </tr>
        <?php
        foreach ($result as $user) {
          echo '<tr>';
          echo '<td>';
                echo $user['name'];
          echo '</td>';
          echo '<td>';
                echo $user['id']. ' ' .@$user['createUserError'];
          echo '</td>';
          echo '<td>';
                foreach ($user['accounts'] as $account){
                   echo $account['orgName']. ' id: '.$account['orgid']. ' '. @$account['createAccountError']. '<br>';
                }
          echo '</td>';
          echo '</tr>';
        }
        ?>
     </table>
<br>
  </form>
  <br>
</div>
<?php include('footer.php') ?>
</body>
</html>