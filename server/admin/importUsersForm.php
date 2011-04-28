<?php include('header.php'); ?>
<body id="edit_user" class="users">
<?php make_navbar('Users') ?>

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
        <b>Always create the organization first before importing users</b>
    </div>
    <div class="sub_header">
        The file should contain the following values seperated by a commas (standard CSV format) (no header).
    </div> 
        <table class="alternate">
        <tr>
          <th>
              Column Name 
          </th>
        </tr>
        <tr> 
          <td>
                Organization Name (The organization must already exist)
          </td>
        </tr>
        <tr>
            <td>
                User Email Address
          </td>
        </tr>
        <tr>
            <td>
                Password (if left blank one will be assigned)
          </td>
        </tr>
        <tr>
            <td>
                First Name
          </td>
        </tr>
        <tr>
            <td>
                Last Name
          </td>
        </tr>
        <tr>
            <td>
                User Role (admin, member, visitor)
          </td>
        </tr>
        <tr>
            <td>
                Comments
          </td>
        </tr>
        <tr>
            <td>
                Send Welcome Email (True, False)
          </td>
        </tr>
     </table>
     
  <div class="sub_header"> 
        Example Data
    </div> 
        <table class="alternate">
        <tr>
          <th>
              Example Data
          </th>
        </tr>
        <tr>
        <td>
            # if you put a # in the first position of a line the system will ignore the entire line
        </td>
        </tr>
        <tr> 
          <td>
                My Company, ron@openqwaq.com, , Ron, Teitelbaum, admin, Batch 101 new hires, True
          </td>
        </tr>
        <tr>
            <td>
                My Company, sam@openqwaq.com, , Sam, Jones, member, Batch 101 new hires, True
          </td>
        </tr>
        <tr>
            <td>
                My Company, VPGerry@openqwaq.com, Gerry123, Gerry, Bossman, admin, VP Account Creation, False
          </td>
        </tr>
     </table>
<br>
  </form>
  <br>
</div>
<?php include('footer.php') ?>
</body>
</html>